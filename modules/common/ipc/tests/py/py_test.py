#!/usr/bin/env python3
"""
Python IPC bindings test.

This test:
1. Creates a virtual environment
2. Installs the generated wheel
3. Tests pub/sub communication using external script files
4. Uninstalls the wheel
5. Deactivates the virtual environment
"""

import sys
import subprocess
import tempfile
import shutil
import glob
import os
from pathlib import Path


class IPCBindingsTest:
    def __init__(self, wheel_path: str):
        """Initialize the test with the wheel path."""
        self.wheel_path = Path(wheel_path)
        self.venv_dir = None
        self.python_exe = None
        self.pip_exe = None
        self.test_passed = False
        self.scripts_dir = Path(__file__).parent

    def create_venv(self) -> bool:
        """Create a virtual environment for testing."""
        try:
            # Create temporary directory for venv
            self.venv_dir = Path(tempfile.mkdtemp(prefix="grape_ipc_test_"))
            print(f"Creating virtual environment at: {self.venv_dir}")
            
            # Create virtual environment
            result = subprocess.run([
                sys.executable, "-m", "venv", str(self.venv_dir)
            ], capture_output=True, text=True)
            
            if result.returncode != 0:
                print(f"Failed to create venv: {result.stderr}")
                return False
                
            # Set up paths for venv executables
            self.python_exe = self.venv_dir / "bin" / "python"
            self.pip_exe = self.venv_dir / "bin" / "pip"
                
            print("Virtual environment created successfully")
            return True
            
        except Exception as e:
            print(f"Error creating virtual environment: {e}")
            return False
    
    def install_wheel(self) -> bool:
        """Install the generated wheel in the virtual environment."""
        try:
            if not self.wheel_path.exists():
                print(f"Wheel not found at: {self.wheel_path}")
                return False
                
            print(f"Installing wheel: {self.wheel_path}")
            
            # Upgrade pip first
            result = subprocess.run([
                str(self.pip_exe), "install", "--upgrade", "pip"
            ], capture_output=True, text=True)
            
            if result.returncode != 0:
                print(f"Failed to upgrade pip: {result.stderr}")
                return False
            
            # Install the wheel
            result = subprocess.run([
                str(self.pip_exe), "install", str(self.wheel_path)
            ], capture_output=True, text=True)
            
            if result.returncode != 0:
                print(f"Failed to install wheel: {result.stderr}")
                return False
                
            print("Wheel installed successfully")
            return True
            
        except Exception as e:
            print(f"Error installing wheel: {e}")
            return False
        
    def test_imports(self) -> bool:
        """Test that the Python bindings can be imported."""
        try:
            import_script_path = self.scripts_dir / "import_test.py"
            
            result = subprocess.run([
                str(self.python_exe), str(import_script_path)
            ], capture_output=True, text=True)
            
            print(result.stdout)
            if result.stderr:
                print(result.stderr)
                
            if result.returncode != 0:
                print("Import test failed")
                return False
                
            print("Import test passed")
            return True
            
        except Exception as e:
            print(f"Error testing imports: {e}")
            return False
    
    def test_pub_sub_communication(self) -> bool:
        """Test publisher-subscriber communication using external script files."""
        try:
            # Get paths to the script files
            publisher_script_path = self.scripts_dir / "publisher.py"
            subscriber_script_path = self.scripts_dir / "subscriber.py"
            
            print("Testing publisher-subscriber communication...")
            
            # Start subscriber
            subscriber_proc = subprocess.Popen([
                str(self.python_exe), str(subscriber_script_path)
            ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            
            # Start publisher
            publisher_proc = subprocess.Popen([
                str(self.python_exe), str(publisher_script_path)
            ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            
            # Wait for publisher and subscriber to complete on their own with a timeout
            try:
                pub_stdout, pub_stderr = publisher_proc.communicate(timeout=10)
                sub_stdout, sub_stderr = subscriber_proc.communicate(timeout=10)
            except subprocess.TimeoutExpired:
                publisher_proc.kill()
                subscriber_proc.kill()
                pub_stdout, pub_stderr = publisher_proc.communicate()
                sub_stdout, sub_stderr = subscriber_proc.communicate()
            
            print("Publisher output:")
            print(pub_stdout)
            if pub_stderr:
                print("Publisher errors:")
                print(pub_stderr)
                
            print("Subscriber output:")
            print(sub_stdout)
            if sub_stderr:
                print("Subscriber errors:")
                print(sub_stderr)
            
            # Check the exit codes and output to determine success
            pub_success = publisher_proc.returncode == 0
            sub_success = subscriber_proc.returncode == 0
            if pub_success and sub_success:
                print("SUCCESS: Publisher-subscriber communication test passed")
                return True
            else:
                print(f"FAILED: Publisher success: {pub_success}, Subscriber success: {sub_success}")
                return False
                
        except Exception as e:
            print(f"Error testing pub-sub communication: {e}")
            return False
    
    def uninstall_wheel(self) -> bool:
        """Uninstall the wheel from the virtual environment."""
        try:
            print("Uninstalling wheel...")
            
            result = subprocess.run([
                str(self.pip_exe), "uninstall", "-y", "grape_ipc_py"
            ], capture_output=True, text=True)
            
            if result.returncode != 0:
                print(f"Failed to uninstall wheel: {result.stderr}")
                return False
                
            print("Wheel uninstalled successfully")
            return True
            
        except Exception as e:
            print(f"Error uninstalling wheel: {e}")
            return False
    
    def cleanup(self) -> bool:
        """Clean up the virtual environment."""
        try:
            if self.venv_dir and self.venv_dir.exists():
                print(f"Removing virtual environment: {self.venv_dir}")
                shutil.rmtree(self.venv_dir)
                print("Virtual environment removed successfully")
            return True
            
        except Exception as e:
            print(f"Error during cleanup: {e}")
            return False
    
    def run_full_test(self) -> bool:
        """Run the complete test suite."""
        try:
            print("=== Starting Python IPC Bindings Integration Test ===")
            
            if not self.create_venv():
                return False
            
            if not self.install_wheel():
                return False
            
            if not self.test_imports():
                return False
            
            if not self.test_pub_sub_communication():
                return False
            
            if not self.uninstall_wheel():
                return False
            
            print("=== All tests passed! ===")
            self.test_passed = True
            return True
            
        except Exception as e:
            print(f"Test suite error: {e}")
            return False
        finally:
            self.cleanup()


def find_latest_wheel(path_arg):
    """
    Find the most recent grape_ipc_py wheel file.
    
    Args:
        path_arg: Either a specific wheel file path or a directory containing wheels
        
    Returns:
        Path to the most recent wheel file
    """
    path = Path(path_arg)
    
    # If the path is a directory, find the newest wheel file
    if path.is_dir():
        pattern = os.path.join(str(path), "grape_ipc_py-*.whl")
        wheels = glob.glob(pattern)
        
        if not wheels:
            print(f"ERROR: No wheel files found matching '{pattern}'")
            sys.exit(1)
            
        # Sort by modification time, newest first
        latest_wheel = max(wheels, key=os.path.getmtime)
        return Path(latest_wheel)
    
    # If it's a file, use it directly
    elif path.is_file():
        return path
    
    # If it doesn't exist
    else:
        print(f"ERROR: The wheel path does not exist: {path}")
        sys.exit(1)

def main():
    if len(sys.argv) != 2:
        print("Usage: python py_test.py <wheel_path_or_dir>")
        sys.exit(1)
    
    # Find the wheel file
    wheel_path = find_latest_wheel(sys.argv[1])
    print(f"Using wheel: {wheel_path}")
    
    # Run the test
    test = IPCBindingsTest(wheel_path)
    
    success = test.run_full_test()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()