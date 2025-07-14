#!/usr/bin/env python3
"""
Item Editor Qt6 - Ubuntu Environment Setup Script
================================================

Professional Ubuntu environment setup script featuring:
- Rich console output with colors and progress bars
- Comprehensive logging to file for debugging
- Automatic installation of all required dependencies
- Qt6 development environment setup
- Python rich library installation
- System package management with error handling

This script prepares an Ubuntu machine for Qt6 Item Editor compilation.
Run this script first, then execute the build script.

Author: Generated for Item Editor Qt6 Project - Ubuntu Setup
"""

import os
import sys
import subprocess
import logging
import time
from datetime import datetime
from pathlib import Path
from typing import List, Optional, Tuple

# Try to import rich, install if not available
try:
    from rich.console import Console
    from rich.panel import Panel
    from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn, TaskProgressColumn
    from rich.table import Table
    from rich.text import Text
    from rich.logging import RichHandler
    from rich.prompt import Confirm
    from rich.status import Status
    from rich.align import Align
    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False
    print("Rich library not found. Installing it first...")


class UbuntuEnvironmentSetup:
    """Professional Ubuntu environment setup class for Qt6 Item Editor project."""
    
    def __init__(self):
        # Initialize console (with or without rich)
        if RICH_AVAILABLE:
            try:
                import locale
                locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')
            except:
                pass
            self.console = Console(force_terminal=True)
        else:
            self.console = None
        
        self.start_time = datetime.now()
        
        # Project paths
        self.project_root = Path(__file__).parent.absolute()
        
        # Setup logging
        self.setup_logging()
        
        # Setup statistics
        self.stats = {
            "packages_installed": 0,
            "python_packages_installed": 0,
            "qt6_verified": False,
            "cmake_verified": False,
            "gcc_verified": False,
            "total_errors": 0,
            "total_warnings": 0,
            "setup_successful": False
        }
    
    def setup_logging(self):
        """Setup comprehensive logging to file and console."""
        # Create logs directory
        log_dir = self.project_root / "logs"
        log_dir.mkdir(exist_ok=True)
        
        # Create log filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = log_dir / f"setup_ubuntu_{timestamp}.log"
        
        # Configure logging
        if RICH_AVAILABLE and self.console:
            logging.basicConfig(
                level=logging.DEBUG,
                format='%(asctime)s - %(levelname)s - %(message)s',
                handlers=[
                    logging.FileHandler(log_file, encoding='utf-8'),
                    RichHandler(console=self.console, show_time=False, show_path=False)
                ]
            )
        else:
            logging.basicConfig(
                level=logging.DEBUG,
                format='%(asctime)s - %(levelname)s - %(message)s',
                handlers=[
                    logging.FileHandler(log_file, encoding='utf-8'),
                    logging.StreamHandler()
                ]
            )
        
        self.logger = logging.getLogger(__name__)
        self.log_file = log_file
        
        self.logger.info(f"Ubuntu environment setup session started - Log file: {log_file}")
        self.logger.info(f"Project directory: {self.project_root}")
    
    def print_message(self, message: str, style: str = "white"):
        """Print message with or without rich formatting."""
        if RICH_AVAILABLE and self.console:
            self.console.print(message, style=style)
        else:
            print(message)
    
    def print_panel(self, content, title: str = "", border_style: str = "blue"):
        """Print panel with or without rich formatting."""
        if RICH_AVAILABLE and self.console:
            self.console.print(Panel(content, title=title, border_style=border_style))
        else:
            print(f"\n=== {title} ===")
            print(content)
            print("=" * (len(title) + 8))
    
    def show_header(self):
        """Display the application header."""
        if RICH_AVAILABLE and self.console:
            header_text = Text()
            header_text.append("Item Editor Qt6", style="bold blue")
            header_text.append(" - ", style="white")
            header_text.append("Ubuntu Environment Setup", style="bold green")
            header_text.append(" [UBUNTU]", style="bold yellow")
            
            header_panel = Panel(
                Align.center(header_text),
                title="[SETUP] Ubuntu Environment Setup",
                title_align="left",
                border_style="blue",
                padding=(1, 2)
            )
            
            self.console.print(header_panel)
            self.console.print()
            
            # Show setup information
            info_table = Table(show_header=False, box=None, padding=(0, 2))
            info_table.add_column("Label", style="cyan", no_wrap=True)
            info_table.add_column("Value", style="white")
            
            info_table.add_row("[DIR] Project Directory:", str(self.project_root))
            info_table.add_row("[LOG] Log File:", str(self.log_file))
            info_table.add_row("[TIME] Started:", self.start_time.strftime("%Y-%m-%d %H:%M:%S"))
            info_table.add_row("[OS] Target Platform:", "Ubuntu Linux")
            
            self.console.print(Panel(info_table, title="[CONFIG] Setup Configuration", border_style="green"))
            self.console.print()
        else:
            print("\n" + "="*60)
            print("Item Editor Qt6 - Ubuntu Environment Setup")
            print("="*60)
            print(f"Project Directory: {self.project_root}")
            print(f"Log File: {self.log_file}")
            print(f"Started: {self.start_time.strftime('%Y-%m-%d %H:%M:%S')}")
            print(f"Target Platform: Ubuntu Linux")
            print("="*60 + "\n")
    
    def run_command(self, command: List[str], description: str, cwd: Optional[Path] = None, 
                   timeout: int = 300) -> Tuple[bool, str, str]:
        """Run a command with logging and output."""
        cmd_str = " ".join(command)
        self.logger.info(f"Running command: {cmd_str}")
        self.logger.info(f"Working directory: {cwd or self.project_root}")
        
        # Show what we're running
        self.print_message(f"[RUNNING] {description}", "bold blue")
        
        try:
            result = subprocess.run(
                command,
                cwd=cwd or self.project_root,
                capture_output=True,
                text=True,
                timeout=timeout
            )
            
            stdout = result.stdout.strip()
            stderr = result.stderr.strip()
            success = result.returncode == 0
            
            # Log the results
            if stdout:
                self.logger.debug(f"STDOUT:\n{stdout}")
            if stderr:
                self.logger.warning(f"STDERR:\n{stderr}")
            
            if success:
                self.print_message(f"[OK] {description}", "green")
                self.logger.info(f"Command succeeded: {cmd_str}")
            else:
                self.print_message(f"[FAIL] {description}", "red")
                self.logger.error(f"Command failed: {cmd_str} (exit code: {result.returncode})")
                self.stats["total_errors"] += 1
            
            return success, stdout, stderr
            
        except subprocess.TimeoutExpired:
            self.print_message(f"[TIMEOUT] {description} - TIMEOUT", "red")
            self.logger.error(f"Command timed out: {cmd_str}")
            self.stats["total_errors"] += 1
            return False, "", "Command timed out"
            
        except Exception as e:
            self.print_message(f"[ERROR] {description} - EXCEPTION", "red")
            self.logger.error(f"Command exception: {cmd_str} - {str(e)}")
            self.stats["total_errors"] += 1
            return False, "", str(e)
    
    def install_python_rich(self) -> bool:
        """Install Python rich library if not already available."""
        if RICH_AVAILABLE:
            self.print_message("[OK] Rich library already available", "green")
            return True
        
        self.print_panel("[PYTHON] Installing Python Rich Library", "Python Setup", "yellow")
        
        # Try pip3 first, then pip
        pip_commands = ["pip3", "pip"]
        
        for pip_cmd in pip_commands:
            success, stdout, stderr = self.run_command(
                [pip_cmd, "install", "rich"], 
                f"Installing rich library with {pip_cmd}",
                timeout=300
            )
            
            if success:
                self.stats["python_packages_installed"] += 1
                self.print_message(f"[OK] Rich library installed successfully with {pip_cmd}", "bold green")
                self.logger.info(f"Rich library installed with {pip_cmd}")
                
                # Try to import rich to verify installation
                try:
                    import rich
                    self.print_message("[OK] Rich library import verified", "green")
                    return True
                except ImportError:
                    self.print_message("[WARN] Rich installed but import failed", "yellow")
                    continue
            else:
                self.print_message(f"[FAIL] Failed to install rich with {pip_cmd}", "red")
        
        # If all pip commands failed, try with --user flag
        for pip_cmd in pip_commands:
            success, stdout, stderr = self.run_command(
                [pip_cmd, "install", "--user", "rich"], 
                f"Installing rich library with {pip_cmd} --user",
                timeout=300
            )
            
            if success:
                self.stats["python_packages_installed"] += 1
                self.print_message(f"[OK] Rich library installed successfully with {pip_cmd} --user", "bold green")
                return True
        
        self.print_message("[FAIL] Failed to install rich library with all methods", "red")
        return False
    
    def check_system_info(self) -> bool:
        """Check Ubuntu system information."""
        self.print_panel("[SYSTEM] Checking System Information", "System Check", "cyan")
        
        # Check Ubuntu version
        success, stdout, stderr = self.run_command(
            ["lsb_release", "-a"], 
            "Checking Ubuntu version"
        )
        
        if success:
            self.print_message("[OK] Ubuntu version information retrieved", "green")
            for line in stdout.split('\n'):
                if line.strip():
                    self.logger.info(f"System info: {line.strip()}")
        else:
            self.print_message("[WARN] Could not retrieve Ubuntu version", "yellow")
            self.stats["total_warnings"] += 1
        
        # Check architecture
        success, stdout, stderr = self.run_command(
            ["uname", "-m"], 
            "Checking system architecture"
        )
        
        if success:
            arch = stdout.strip()
            self.print_message(f"[OK] System architecture: {arch}", "green")
            self.logger.info(f"System architecture: {arch}")
        
        return True
    
    def update_package_lists(self) -> bool:
        """Update Ubuntu package lists."""
        self.print_panel("[APT] Updating Package Lists", "Package Management", "yellow")
        
        success, stdout, stderr = self.run_command(
            ["sudo", "apt", "update"], 
            "Updating package lists",
            timeout=300
        )
        
        if success:
            self.print_message("[OK] Package lists updated successfully", "bold green")
            return True
        else:
            self.print_message("[FAIL] Failed to update package lists", "red")
            return False
    
    def install_system_packages(self) -> bool:
        """Install required system packages for Qt6 development."""
        self.print_panel("[PACKAGES] Installing System Packages", "Package Installation", "blue")
        
        # Essential packages for Qt6 development on Ubuntu
        packages = [
            # Build tools
            "build-essential", "cmake", "git", "pkg-config", "ninja-build",
            
            # Qt6 development packages
            "qt6-base-dev", "qt6-tools-dev", "qt6-tools-dev-tools",
            "libqt6core6", "libqt6gui6", "libqt6widgets6", "libqt6network6", "libqt6xml6",
            "qt6-qpa-plugins",
            
            # Graphics and X11 dependencies
            "libgl1-mesa-dev", "libglu1-mesa-dev", "libxkbcommon-dev",
            "libxcb-xinerama0-dev", "libxcb-cursor-dev", "libfontconfig1-dev",
            "libfreetype6-dev", "libx11-dev", "libxext-dev", "libxfixes-dev",
            "libxi-dev", "libxrender-dev", "libxcb1-dev", "libx11-xcb-dev",
            "libxcb-glx0-dev", "libxcb-keysyms1-dev", "libxcb-image0-dev",
            "libxcb-shm0-dev", "libxcb-icccm4-dev", "libxcb-sync-dev",
            "libxcb-xfixes0-dev", "libxcb-shape0-dev", "libxcb-randr0-dev",
            "libxcb-render-util0-dev"
        ]
        
        # Install packages with progress tracking
        if RICH_AVAILABLE and self.console:
            try:
                with Progress(
                    SpinnerColumn(),
                    TextColumn("[progress.description]{task.description}"),
                    BarColumn(),
                    TaskProgressColumn(),
                    console=self.console
                ) as progress:
                    
                    # Install in chunks to avoid timeout
                    chunk_size = 10
                    total_chunks = (len(packages) + chunk_size - 1) // chunk_size
                    install_task = progress.add_task("Installing packages...", total=total_chunks)
                    
                    for i in range(0, len(packages), chunk_size):
                        chunk = packages[i:i + chunk_size]
                        chunk_num = i // chunk_size + 1
                        
                        progress.update(install_task, description=f"Installing chunk {chunk_num}/{total_chunks}...")
                        
                        cmd = ["sudo", "apt", "install", "-y"] + chunk
                        success, stdout, stderr = self.run_command(
                            cmd, 
                            f"Installing package chunk {chunk_num}",
                            timeout=600  # 10 minute timeout for package installation
                        )
                        
                        if not success:
                            self.print_message(f"[FAIL] Failed to install package chunk: {chunk}", "red")
                            return False
                        
                        progress.advance(install_task)
            except Exception as e:
                self.print_message(f"[FAIL] Exception during package installation: {e}", "red")
                self.logger.error(f"Exception during package installation: {e}")
                return False
        else:
            # Fallback without rich progress
            chunk_size = 10
            total_chunks = (len(packages) + chunk_size - 1) // chunk_size
            
            for i in range(0, len(packages), chunk_size):
                chunk = packages[i:i + chunk_size]
                chunk_num = i // chunk_size + 1
                
                print(f"Installing package chunk {chunk_num}/{total_chunks}...")
                
                cmd = ["sudo", "apt", "install", "-y"] + chunk
                success, stdout, stderr = self.run_command(
                    cmd, 
                    f"Installing package chunk {chunk_num}",
                    timeout=600
                )
                
                if not success:
                    self.print_message(f"[FAIL] Failed to install package chunk: {chunk}", "red")
                    return False
        
        self.stats["packages_installed"] = len(packages)
        self.print_message(f"[OK] Successfully installed {len(packages)} packages", "bold green")
        self.logger.info(f"Successfully installed {len(packages)} packages")
        return True
    
    def verify_installations(self) -> bool:
        """Verify that all required tools are properly installed."""
        self.print_panel("[VERIFY] Verifying Installations", "Verification", "cyan")
        
        verifications = []
        
        # Check CMake
        success, stdout, stderr = self.run_command(
            ["cmake", "--version"], 
            "Verifying CMake installation"
        )
        verifications.append(("CMake", success, stdout.split('\n')[0] if success and stdout else "Not found"))
        if success:
            self.stats["cmake_verified"] = True
        
        # Check GCC
        success, stdout, stderr = self.run_command(
            ["g++", "--version"], 
            "Verifying GCC installation"
        )
        verifications.append(("GCC", success, stdout.split('\n')[0] if success and stdout else "Not found"))
        if success:
            self.stats["gcc_verified"] = True
        
        # Check qmake6
        success, stdout, stderr = self.run_command(
            ["/usr/lib/qt6/bin/qmake6", "-version"], 
            "Verifying Qt6 installation"
        )
        verifications.append(("Qt6 qmake", success, stdout.split('\n')[0] if success and stdout else "Not found"))
        if success:
            self.stats["qt6_verified"] = True
        
        # Check pkg-config
        success, stdout, stderr = self.run_command(
            ["pkg-config", "--version"], 
            "Verifying pkg-config installation"
        )
        verifications.append(("pkg-config", success, stdout.strip() if success and stdout else "Not found"))
        
        # Display verification results
        if RICH_AVAILABLE and self.console:
            table = Table(show_header=True, header_style="bold magenta")
            table.add_column("Tool", style="cyan")
            table.add_column("Status", justify="center")
            table.add_column("Version/Details", style="dim")
            
            all_verified = True
            for name, status, details in verifications:
                if status:
                    table.add_row(name, "[OK] OK", details)
                else:
                    table.add_row(name, "[FAIL] FAIL", details)
                    all_verified = False
            
            self.console.print(table)
            self.console.print()
        else:
            print("\nVerification Results:")
            print("-" * 50)
            all_verified = True
            for name, status, details in verifications:
                status_str = "[OK]" if status else "[FAIL]"
                print(f"{name:15} {status_str:8} {details}")
                if not status:
                    all_verified = False
            print("-" * 50)
        
        if all_verified:
            self.print_message("[OK] All tools verified successfully", "bold green")
            return True
        else:
            self.print_message("[FAIL] Some tools failed verification", "red")
            return False
    
    def show_setup_summary(self):
        """Display a comprehensive setup summary."""
        end_time = datetime.now()
        duration = end_time - self.start_time
        
        # Create summary
        if self.stats["setup_successful"]:
            summary_style = "bold green"
            summary_text = "[SUCCESS] ENVIRONMENT SETUP COMPLETED SUCCESSFULLY!"
            border_style = "green"
        else:
            summary_style = "bold red"
            summary_text = "[FAIL] ENVIRONMENT SETUP FAILED"
            border_style = "red"
        
        if RICH_AVAILABLE and self.console:
            # Build statistics table
            stats_table = Table(show_header=False, box=None, padding=(0, 2))
            stats_table.add_column("Metric", style="cyan", no_wrap=True)
            stats_table.add_column("Value", style="white")
            
            stats_table.add_row("[TIME] Duration:", f"{duration.total_seconds():.1f} seconds")
            stats_table.add_row("[PKG] System Packages:", str(self.stats["packages_installed"]))
            stats_table.add_row("[PY] Python Packages:", str(self.stats["python_packages_installed"]))
            stats_table.add_row("[CMAKE] CMake:", "[OK] Verified" if self.stats["cmake_verified"] else "[FAIL] Failed")
            stats_table.add_row("[GCC] GCC Compiler:", "[OK] Verified" if self.stats["gcc_verified"] else "[FAIL] Failed")
            stats_table.add_row("[QT6] Qt6:", "[OK] Verified" if self.stats["qt6_verified"] else "[FAIL] Failed")
            stats_table.add_row("[WARN] Warnings:", str(self.stats["total_warnings"]))
            stats_table.add_row("[ERROR] Errors:", str(self.stats["total_errors"]))
            
            self.console.print()
            self.console.print(Panel(
                Text(summary_text, style=summary_style),
                title="[SUMMARY] Setup Summary",
                border_style=border_style,
                padding=(1, 2)
            ))
            
            self.console.print(Panel(stats_table, title="[STATS] Setup Statistics", border_style=border_style))
            
            if self.stats["setup_successful"]:
                # Show next steps
                next_steps = Text()
                next_steps.append("[READY] Environment is ready for compilation!\n\n", style="bold green")
                next_steps.append("[NEXT] Next steps:\n", style="bold blue")
                next_steps.append("   1. Run the build script: python3 build_linux_all_in_one.py\n", style="white")
                next_steps.append("   2. Or use manual CMake build process\n", style="white")
                next_steps.append("   3. All dependencies are now installed\n\n", style="white")
                next_steps.append("[LOG] Setup log saved to:\n", style="bold blue")
                next_steps.append(f"   {self.log_file}\n", style="white")
                
                self.console.print(Panel(next_steps, title="[NEXT] Next Steps", border_style="blue"))
        else:
            print(f"\n{summary_text}")
            print(f"Duration: {duration.total_seconds():.1f} seconds")
            print(f"System Packages: {self.stats['packages_installed']}")
            print(f"Python Packages: {self.stats['python_packages_installed']}")
            print(f"CMake: {'Verified' if self.stats['cmake_verified'] else 'Failed'}")
            print(f"GCC: {'Verified' if self.stats['gcc_verified'] else 'Failed'}")
            print(f"Qt6: {'Verified' if self.stats['qt6_verified'] else 'Failed'}")
            print(f"Warnings: {self.stats['total_warnings']}")
            print(f"Errors: {self.stats['total_errors']}")
            
            if self.stats["setup_successful"]:
                print("\nEnvironment is ready for compilation!")
                print("Next steps:")
                print("  1. Run the build script: python3 build_linux_all_in_one.py")
                print("  2. Or use manual CMake build process")
                print("  3. All dependencies are now installed")
            
            print(f"\nSetup log saved to: {self.log_file}")
        
        # Log final summary
        self.logger.info(f"Setup completed in {duration.total_seconds():.1f} seconds")
        self.logger.info(f"Final status: {'SUCCESS' if self.stats['setup_successful'] else 'FAILED'}")
        self.logger.info(f"Log file saved to: {self.log_file}")
    
    def run_setup(self) -> bool:
        """Run the complete environment setup process."""
        try:
            self.show_header()
            
            # Step 1: Install Python rich library
            if not self.install_python_rich():
                self.print_message("[WARN] Rich library installation failed, continuing with basic output", "yellow")
                self.stats["total_warnings"] += 1
            
            # Step 2: Check system information
            if not self.check_system_info():
                return False
            
            # Step 3: Update package lists
            if not self.update_package_lists():
                return False
            
            # Step 4: Install system packages
            if not self.install_system_packages():
                return False
            
            # Step 5: Verify installations
            if not self.verify_installations():
                return False
            
            self.stats["setup_successful"] = True
            return True
            
        except KeyboardInterrupt:
            self.print_message("\n[STOP] Setup interrupted by user", "bold red")
            self.logger.warning("Setup interrupted by user")
            return False
        except Exception as e:
            self.print_message(f"\n[ERROR] Unexpected error: {e}", "bold red")
            self.logger.error(f"Unexpected error: {e}")
            return False
        finally:
            self.show_setup_summary()


def main():
    """Main entry point."""
    # Check if running on Linux
    if sys.platform != "linux":
        print("[WARN] This script is optimized for Ubuntu Linux")
        response = input("Continue anyway? (y/N): ")
        if response.lower() not in ['y', 'yes']:
            sys.exit(1)
    
    # Create and run setup
    setup = UbuntuEnvironmentSetup()
    success = setup.run_setup()
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()