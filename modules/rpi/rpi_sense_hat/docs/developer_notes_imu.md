# Developer notes: Reading IMU data from Sense HAT v2 on the Raspberry Pi

## Device identification

The Sense HAT connects to the Pi via I2C. Scan the bus to detect devices

```bash
# Install tools
sudo apt install i2c-tools

# Sense HAT typically uses I2C bus 1
sudo i2cdetect -y 1

# Get info on specific device
cat /sys/class/i2c-dev/i2c-1/device/1-006a/uevent # accel/gyro
cat /sys/class/i2c-dev/i2c-1/device/1-001c/uevent # magnetometer

# Read the WHO_AM_I register
# Replace 0x6a with whichever address showed up in i2cdetect
# Register 0x0F is WHO_AM_I for most ST IMUs
sudo i2cget -y 1 0x6a 0x0f

# If there is a magnetometer at a separate address:
sudo i2cget -y 1 0x1c 0x0f
```

We should expect to see 

Address | WHO_AM_I | Chip        | Function 
--------|----------|-------------|---------
`0x6a`  | `0x68`   | **LSM9DS1** | Accelerometer + Gyroscope 
`0x1c`  | `0x3D`   | **LSM9DS1** | Magnetometer 

The other addresses on the bus are the rest of the Sense HAT:

Address     | Device
------------|---
`0x39`      | APDS-9960 (gesture/light/proximity) 
`0x46` (UU) | Sense HAT microcontroller (kernel driver bound — hence `UU`) 
`0x5c`      | LPS25H (pressure/temperature) 
`0x5f`      | HTS221 (humidity/temperature) 

## LSM9DS1 key specifications

Parameter                | Value 
-------------------------|----------
Accel max ODR            | **952 Hz** 
Gyro max ODR             | **952 Hz** 
Mag max ODR              | **80 Hz** 
Hardware FIFO            | **32 samples** (accel+gyro only, 33ms of data at 952Hz)
FIFO modes               | Bypass, FIFO, Continuous, Continuous-to-FIFO 
FIFO threshold interrupt | Yes (INT1/INT2 configurable) 
Interface                | I2C (on Sense HAT, **not** SPI) 

- The magnetometer is a separate device from accel/gyro, has no FIFO and has a much slower sampling rate
- The IMU is hard wired as an i2c device (via `/dev/i2c-1`). We will poll to read; the complexity of setting up DMA is not worth it. DMA may be a better approach for industrial equivalent (ISM series) at can be sampled at up to 6kHz. In this case, it would be better to rely on IIO or SPI (and spidev) for setting up DMA easily.  

## Timing budget analysis

At 952 Hz ODR, we have ~1050 µs per sample. Theoretical I2C transactions cost:

Operation              | Bytes on wire                  | Time at 400 kHz I2C 
-----------------------|--------------------------------|---
Read 6 bytes (gyro)    | ~10 bytes with addressing/ACKs | ~200 µs 
Read 6 bytes (accel)   | ~10 bytes with addressing/ACKs | ~200 µs 
Read FIFO_SRC register | ~3 bytes                       | ~60 µs 
**Total per sample**   | | **~460 µs** 

That leaves ~590 µs for AHRS computations not including jitter from the syscalls. On a PREEMPT_RT kernel with an isolated CPU, `ioctl`/`read`/`write` on `/dev/i2c-1` may have worst-case latency in the low tens of microseconds for the syscall overhead itself (the I2C bus time is fixed).

For even more headroom, the LSM9DS1 also supports 476 Hz ODR, giving ~2100 µs per sample. Even with magnetometer reads (another ~200 µs at 80 Hz, read every ~12th iteration), that leaves plenty of room for AHRS calculations

The I2C clock speed is critical. At 100 kHz (default on many Pi configs), the per-sample I2C time quadruples to ~1840 µs, exceeding your 1050 µs budget. Therefore, set the speed to 400 kHz in the boot config:

```bash name
# In /boot/config.txt or /boot/firmware/config.txt:
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

Reboot for this to take effect. Verify as follows

```bash
# Should show: 00 06 1a 80  (= 0x00061A80 = 400000)
hexdump -C /sys/firmware/devicetree/base/soc/i2c@7e804000/clock-frequency

# Run an empirical test (should be about 2.6s)
time for i in $(seq 1 100); do sudo i2cget -y 1 0x6a 0x0f > /dev/null; done

# Confirm the IMU still responds (should still return 0x68)
sudo i2cget -y 1 0x6a 0x0f
```

Empirical test should result in ~2.6s. The `i2cget` command-line tool's overhead is dominated by process forking, shell execution, and userspace `open()/close()` of `/dev/i2c-1` per invocation — not the I2C bus time. The bus speed improvement won't show up in this test. The following test opens the device once and does repeated reads of accel/gyro without process respawn:

```c
// Timing test with IMU actually powered on and producing data
// Compile: gcc -O2 -o i2c_timing_test_v2 i2c_timing_test_v2.c
// Run:     sudo ./i2c_timing_test_v2

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

static int i2c_write_reg(int fd, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val };
    return write(fd, buf, 2);
}

static uint8_t i2c_read_reg(int fd, uint8_t reg) {
    uint8_t val = 0;
    write(fd, &reg, 1);
    read(fd, &val, 1);
    return val;
}

static int i2c_read_burst(int fd, uint8_t start_reg, uint8_t *buf, int len) {
    // Use i2c_rdwr_ioctl for a proper repeated-start burst read
    // This is how a real driver does it — single atomic transaction
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data rdwr;

    uint8_t reg = start_reg;

    msgs[0].addr = 0x6a;
    msgs[0].flags = 0;          // write
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = 0x6a;
    msgs[1].flags = I2C_M_RD;  // read
    msgs[1].len = len;
    msgs[1].buf = buf;

    rdwr.msgs = msgs;
    rdwr.nmsgs = 2;

    return ioctl(fd, I2C_RDWR, &rdwr);
}

int main(void) {
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    if (ioctl(fd, I2C_SLAVE, 0x6a) < 0) { perror("ioctl"); return 1; }

    // Confirm chip identity
    printf("WHO_AM_I = 0x%02x\n", i2c_read_reg(fd, 0x0f));

    // Power on gyro at 952 Hz, 245 dps
    // CTRL_REG1_G (0x10): ODR=952Hz (110), FS=245dps (00), BW=00
    i2c_write_reg(fd, 0x10, 0xC0);

    // Power on accel at 952 Hz, +/-2g
    // CTRL_REG6_XL (0x20): ODR=952Hz (110), FS=2g (00), BW=auto
    i2c_write_reg(fd, 0x20, 0xC0);

    // Wait for sensor to stabilise
    usleep(100000);  // 100ms

    // Verify data is flowing: read STATUS_REG (0x17)
    uint8_t status = i2c_read_reg(fd, 0x17);
    printf("STATUS_REG = 0x%02x (expect bits 0,1 set = new accel+gyro data)\n", status);

    // ---- Test 1: Burst read using write+read (original method) ----
    {
        const int N = 1000;
        uint8_t buf[6];
        uint8_t reg_g = 0x18;
        uint8_t reg_a = 0x28;
        struct timespec start, end;

        // Warm up
        write(fd, &reg_g, 1); read(fd, buf, 6);
        write(fd, &reg_a, 1); read(fd, buf, 6);

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < N; i++) {
            write(fd, &reg_g, 1);
            read(fd, buf, 6);
            write(fd, &reg_a, 1);
            read(fd, buf, 6);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 +
                            (end.tv_nsec - start.tv_nsec) / 1e3;
        printf("\n--- Method 1: separate write()/read() ---\n");
        printf("%d 6-axis reads in %.1f us (avg: %.1f us)\n",
               N, elapsed_us, elapsed_us / N);
    }

    // ---- Test 2: Burst read using I2C_RDWR ioctl (proper repeated start) ----
    {
        const int N = 1000;
        uint8_t gbuf[6], abuf[6];
        struct timespec start, end;

        // Warm up
        i2c_read_burst(fd, 0x18, gbuf, 6);
        i2c_read_burst(fd, 0x28, abuf, 6);

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < N; i++) {
            i2c_read_burst(fd, 0x18, gbuf, 6);
            i2c_read_burst(fd, 0x28, abuf, 6);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 +
                            (end.tv_nsec - start.tv_nsec) / 1e3;
        printf("\n--- Method 2: I2C_RDWR ioctl (repeated start) ---\n");
        printf("%d 6-axis reads in %.1f us (avg: %.1f us)\n",
               N, elapsed_us, elapsed_us / N);

        // Print a sample to confirm real data
        int16_t gx = (int16_t)(gbuf[0] | (gbuf[1] << 8));
        int16_t gy = (int16_t)(gbuf[2] | (gbuf[3] << 8));
        int16_t gz = (int16_t)(gbuf[4] | (gbuf[5] << 8));
        int16_t ax = (int16_t)(abuf[0] | (abuf[1] << 8));
        int16_t ay = (int16_t)(abuf[2] | (abuf[3] << 8));
        int16_t az = (int16_t)(abuf[4] | (abuf[5] << 8));
        printf("Last sample: gyro=(%d,%d,%d) accel=(%d,%d,%d)\n",
               gx, gy, gz, ax, ay, az);
    }

    // Power down
    i2c_write_reg(fd, 0x10, 0x00);
    i2c_write_reg(fd, 0x20, 0x00);
    close(fd);

    printf("\nBudget at 952Hz = 1050 us per sample\n");
    return 0;
}
```

This test:
1. **Powers on** the gyro and accel at 952 Hz before measuring
2. **Verifies** the STATUS register shows new data available
3. Tests **two I2C methods** — the original separate `write()`/`read()` vs. the more correct `I2C_RDWR` ioctl with repeated-start condition
4. **Prints actual sensor values** to confirm real data, not zeros

We want `I2C_RDWR` - it's a single ioctl per burst read (one syscall instead of two), and uses a proper I2C repeated-start condition which the LSM9DS1 expects for atomic multi-byte reads.

Running the test should show something like the following
```bash
./i2c_timing_test_v2 
WHO_AM_I = 0x68
STATUS_REG = 0x07 (expect bits 0,1 set = new accel+gyro data)

--- Method 1: separate write()/read() ---
1000 6-axis reads in 469496.4 us (avg: 469.5 us)

--- Method 2: I2C_RDWR ioctl (repeated start) ---
1000 6-axis reads in 448916.8 us (avg: 448.9 us)
Last sample: gyro=(154,128,431) accel=(-309,-14,15805)
```

The following does a similar timing test on the magnetometer

```c
// Measure magnetometer read timing
// Compile: gcc -O2 -o mag_timing_test mag_timing_test.c
// Run:     sudo ./mag_timing_test

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <time.h>
#include <stdint.h>

static void i2c_write_reg(int fd, uint8_t addr, uint8_t reg, uint8_t val) {
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data rdwr;
    uint8_t buf[2] = { reg, val };
    msg.addr = addr;
    msg.flags = 0;
    msg.len = 2;
    msg.buf = buf;
    rdwr.msgs = &msg;
    rdwr.nmsgs = 1;
    ioctl(fd, I2C_RDWR, &rdwr);
}

static uint8_t i2c_read_reg(int fd, uint8_t addr, uint8_t reg) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data rdwr;
    uint8_t val = 0;
    msgs[0].addr = addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;
    msgs[1].addr = addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 1;
    msgs[1].buf = &val;
    rdwr.msgs = msgs;
    rdwr.nmsgs = 2;
    ioctl(fd, I2C_RDWR, &rdwr);
    return val;
}

static int i2c_read_burst(int fd, uint8_t addr, uint8_t start_reg, uint8_t *buf, int len) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data rdwr;
    // LSM9DS1 mag: bit 7 of sub-address must be set for auto-increment
    uint8_t reg = start_reg | 0x80;
    msgs[0].addr = addr;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;
    msgs[1].addr = addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = len;
    msgs[1].buf = buf;
    rdwr.msgs = msgs;
    rdwr.nmsgs = 2;
    return ioctl(fd, I2C_RDWR, &rdwr);
}

int main(void) {
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    // Verify mag identity
    // LSM9DS1 mag WHO_AM_I is at register 0x0F, address 0x1C
    printf("Mag WHO_AM_I = 0x%02x (expect 0x3d)\n", i2c_read_reg(fd, 0x1c, 0x0f));

    // Configure magnetometer for 80 Hz, continuous mode
    // CTRL_REG1_M (0x20): TEMP_COMP=1, XY_PERF=11(ultra-high), ODR=111(80Hz), FAST_ODR=0, ST=0
    i2c_write_reg(fd, 0x1c, 0x20, 0xFC);
    // CTRL_REG2_M (0x21): FS=00 (+/-4 gauss), default rest
    i2c_write_reg(fd, 0x1c, 0x21, 0x00);
    // CTRL_REG3_M (0x22): Continuous conversion mode (bits 1:0 = 00)
    i2c_write_reg(fd, 0x1c, 0x22, 0x00);
    // CTRL_REG4_M (0x23): Z_PERF=11(ultra-high)
    i2c_write_reg(fd, 0x1c, 0x23, 0x0C);

    usleep(100000);  // 100ms stabilise

    uint8_t status = i2c_read_reg(fd, 0x1c, 0x27);
    printf("Mag STATUS_REG = 0x%02x (expect bits 0-2 set + bit 3 = new XYZ data)\n", status);

    // Also power on accel/gyro at 476 Hz so we can test the combined timing
    // CTRL_REG1_G (0x10): ODR=476Hz (101), FS=245dps (00)
    i2c_write_reg(fd, 0x6a, 0x10, 0xA0);
    // CTRL_REG6_XL (0x20): ODR=476Hz (101), FS=2g (00)
    i2c_write_reg(fd, 0x6a, 0x20, 0xA0);
    usleep(100000);

    // ---- Test 1: Magnetometer read only ----
    {
        const int N = 1000;
        uint8_t mbuf[6];
        struct timespec start, end;

        i2c_read_burst(fd, 0x1c, 0x28, mbuf, 6);  // warm up

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < N; i++) {
            i2c_read_burst(fd, 0x1c, 0x28, mbuf, 6);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 +
                            (end.tv_nsec - start.tv_nsec) / 1e3;
        printf("\n--- Magnetometer only ---\n");
        printf("%d reads in %.1f us (avg: %.1f us)\n", N, elapsed_us, elapsed_us / N);

        int16_t mx = (int16_t)(mbuf[0] | (mbuf[1] << 8));
        int16_t my = (int16_t)(mbuf[2] | (mbuf[3] << 8));
        int16_t mz = (int16_t)(mbuf[4] | (mbuf[5] << 8));
        printf("Last sample: mag=(%d,%d,%d)\n", mx, my, mz);
    }

    // ---- Test 2: Full 9-axis (gyro + accel + mag) combined ----
    {
        const int N = 1000;
        uint8_t gbuf[6], abuf[6], mbuf[6];
        struct timespec start, end;

        // warm up
        i2c_read_burst(fd, 0x6a, 0x18, gbuf, 6);
        i2c_read_burst(fd, 0x6a, 0x28, abuf, 6);
        i2c_read_burst(fd, 0x1c, 0x28, mbuf, 6);

        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < N; i++) {
            i2c_read_burst(fd, 0x6a, 0x18, gbuf, 6);
            i2c_read_burst(fd, 0x6a, 0x28, abuf, 6);
            i2c_read_burst(fd, 0x1c, 0x28, mbuf, 6);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed_us = (end.tv_sec - start.tv_sec) * 1e6 +
                            (end.tv_nsec - start.tv_nsec) / 1e3;
        printf("\n--- Full 9-axis (gyro + accel + mag) ---\n");
        printf("%d reads in %.1f us (avg: %.1f us)\n", N, elapsed_us, elapsed_us / N);
        printf("Budget at 476Hz = 2100 us per sample\n");
        printf("Remaining for AHRS: %.1f us\n", 2100.0 - elapsed_us / N);
    }

    // Power down
    i2c_write_reg(fd, 0x6a, 0x10, 0x00);
    i2c_write_reg(fd, 0x6a, 0x20, 0x00);
    i2c_write_reg(fd, 0x1c, 0x22, 0x03);  // mag power-down

    close(fd);
    return 0;
}
```

Running the test should show something like the following

```bash
./mag_timing_test 
Mag WHO_AM_I = 0x3d (expect 0x3d)
Mag STATUS_REG = 0xff (expect bits 0-2 set + bit 3 = new XYZ data)

--- Magnetometer only ---
1000 reads in 224551.4 us (avg: 224.6 us)
Last sample: mag=(765,-661,-5562)

--- Full 9-axis (gyro + accel + mag) ---
1000 reads in 676281.7 us (avg: 676.3 us)
Budget at 476Hz = 2100 us per sample
Remaining for AHRS: 1423.7 us
```

Summary of timing budget at 476 Hz

Operation                           | Measured Time | Notes
------------------------------------|---------------|------
6-axis (gyro+accel) via `I2C_RDWR`  | **449 µs**    | Every iteration 
Magnetometer via `I2C_RDWR`         | **225 µs**    | Every 6th iteration 
**Full 9-axis**                     | **676 µs**    | Worst-case iteration 
**Sample period at 476 Hz**         | **2100 µs**   | 
**Remaining for AHRS (worst case)** | **1424 µs**   | 

## Next Steps

- **Profile jitter** — add the `Profiler` class from the grape thread example to measure actual `dt` statistics and confirm worst-case is within budget
- **Calibration** — the raw counts need scale factors (gyro: 8.75 mdps/LSB at 245 dps, accel: 0.061 mg/LSB at ±2g, mag: 0.14 mgauss/LSB at ±4 gauss) — do this in the AHRS or as a conversion step before it
