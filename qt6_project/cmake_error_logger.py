#!/usr/bin/env python3
"""
CMake Error Logger - Captures errors from full build process
Runs both CMake configure and build to capture all compilation errors
from the 3 libraries and executable.
"""

import subprocess
import os
import sys
from datetime import datetime

def run_full_cmake_build_and_log_errors():
    """Run full CMake configure + build and capture all errors to log file."""
    
    # Create logs directory if it doesn't exist
    os.makedirs("logs", exist_ok=True)
    
    # Generate log filename with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = f"logs/cmake_build_errors_{timestamp}.log"
    
    with open(log_file, 'w') as f:
        f.write(f"CMake Full Build Error Log - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write("=" * 80 + "\n\n")
        
        # Step 1: CMake Configure
        f.write("STEP 1: CMAKE CONFIGURE\n")
        f.write("-" * 40 + "\n")
        
        cmake_configure_cmd = [
            "cmake", "-B", "build", "-S", ".", 
            "-DCMAKE_PREFIX_PATH=/usr/lib/qt6"
        ]
        
        f.write(f"Command: {' '.join(cmake_configure_cmd)}\n\n")
        
        try:
            result = subprocess.run(
                cmake_configure_cmd,
                capture_output=True,
                text=True,
                cwd=os.getcwd()
            )
            
            f.write(f"Configure Return code: {result.returncode}\n\n")
            
            if result.stderr:
                f.write("Configure STDERR:\n")
                f.write(result.stderr)
                f.write("\n")
            
            if result.stdout:
                f.write("Configure STDOUT:\n")
                f.write(result.stdout)
                f.write("\n")
                
        except Exception as e:
            f.write(f"Configure Exception: {str(e)}\n\n")
            
        f.write("\n" + "=" * 80 + "\n\n")
        
        # Step 2: CMake Build (compile all libraries and executable)
        f.write("STEP 2: CMAKE BUILD (Libraries + Executable)\n")
        f.write("-" * 40 + "\n")
        
        cmake_build_cmd = [
            "cmake", "--build", "build", "--parallel", "4"
        ]
        
        f.write(f"Command: {' '.join(cmake_build_cmd)}\n\n")
        
        try:
            result = subprocess.run(
                cmake_build_cmd,
                capture_output=True,
                text=True,
                cwd=os.getcwd()
            )
            
            f.write(f"Build Return code: {result.returncode}\n\n")
            
            if result.stderr:
                f.write("Build STDERR:\n")
                f.write(result.stderr)
                f.write("\n")
            
            if result.stdout:
                f.write("Build STDOUT:\n")
                f.write(result.stdout)
                f.write("\n")
                
        except Exception as e:
            f.write(f"Build Exception: {str(e)}\n\n")
    
    print(f"Full CMake build errors logged to: {log_file}")

if __name__ == "__main__":
    run_full_cmake_build_and_log_errors()