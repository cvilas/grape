# ahrs - Attitude-Heading Reference System using Raspberry Pi5 Sense HAT

## TODO

- [ ] Pick an AHRS algorithm to implement
  - [ ] Implement constexpr matrix and quaternion operations library to support AHRS implementation
    - [ ] [refx](https://github.com/mosaico-labs/refx)
    - [ ] [PoC](https://github.com/cvilas/scratch/blob/master/linalg.cpp)
  - [ ] Implement the algorithm
- [ ] 3D viewing and signal plotting (SDL3, imgui, implot)

## Make hat work in Ubuntu

- sense hat IMU: https://github.com/astro-pi/python-sense-hat/issues/79. And try linux-modules-extra-raspi

## References

- AHRS descriptions: https://ahrs.readthedocs.io/en/latest/index.html
- AHRS Reference implementations: https://github.com/CCNYRoboticsLab/imu_tools
- x-io technologies: [[product](https://x-io.co.uk/x-imu3/)] [[code](https://github.com/xioTechnologies)]
