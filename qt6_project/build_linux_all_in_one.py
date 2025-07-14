#!/usr/bin/env python3
"""
Item Editor Qt6 - Linux Build Script (All-in-One) with Rich Graphics and Logging
===============================================================================

Professional Linux build script featuring:
- Rich console output with colors and progress bars
- Comprehensive logging to file for debugging
- Better error handling and diagnostics
- Linux-specific optimizations and package management
- Detailed build verification and deployment
- Beautiful visual feedback and professional presentation

Author: Generated for Item Editor Qt6 Project - Linux Edition
"""

import os
import sys
import subprocess
import logging
import multiprocessing
import shutil
import time
from datetime import datetime
from pathlib import Path
from typing import List, Optional, Tuple

try:
    from rich.console import Console
    from rich.panel import Panel
    from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn, TaskProgressColumn
    from rich.table import Table
    from rich.text import Text
    from rich.logging import RichHandler
    from rich.prompt import Confirm
    from rich.tree import Tree
    from rich.layout import Layout
    from rich.live import Live
    from rich.align import Align
    from rich.columns import Columns
    from rich.status import Status
except ImportError:
    print("ERROR: 'rich' library not found!")
    print("Please install it with: pip install rich")
    print("This script requires the 'rich' library for enhanced console output.")
    sys.exit(1)


class LinuxBuilder:
    """Professional Linux builder class for Qt6 Item Editor project with rich UI."""
    
    def __init__(self):
        # Set up console with proper encoding
        try:
            import locale
            locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')
        except:
            pass
        
        # Initialize rich console
        self.console = Console(force_terminal=True)
        self.start_time = datetime.now()
        
        # Project paths
        self.project_root = Path(__file__).parent.absolute()
        self.build_dir = self.project_root / "build"
        self.deploy_dir = self.project_root / "deploy_linux"
        
        # Setup logging
        self.setup_logging()
        
        # Build configuration
        self.plugins = ["PluginOne", "PluginTwo", "PluginThree"]
        self.build_config = "Release"
        
        # Build statistics
        self.stats = {
            "packages_installed": 0,
            "plugins_built": 0,
            "plugins_failed": 0,
            "main_app_built": False,
            "deployment_successful": False,
            "build_successful": False,
            "cmake_successful": False,
            "total_errors": 0,
            "total_warnings": 0,
            "qt6_version": "Unknown"
        }
    
    def setup_logging(self):
        """Setup comprehensive logging to file and console with rich formatting."""
        # Create logs directory
        log_dir = self.project_root / "logs"
        log_dir.mkdir(exist_ok=True)
        
        # Create log filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = log_dir / f"build_linux_{timestamp}.log"
        
        # Configure logging with rich handler
        logging.basicConfig(
            level=logging.DEBUG,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file, encoding='utf-8'),
                RichHandler(console=self.console, show_time=False, show_path=False)
            ]
        )
        
        self.logger = logging.getLogger(__name__)
        self.log_file = log_file
        
        self.logger.info(f"Linux build session started - Log file: {log_file}")
        self.logger.info(f"Project directory: {self.project_root}")
    
    def show_header(self):
        """Display the application header with rich formatting."""
        header_text = Text()
        header_text.append("Item Editor Qt6", style="bold blue")
        header_text.append(" - ", style="white")
        header_text.append("Linux Build Script", style="bold green")
        header_text.append(" [LINUX]", style="bold yellow")
        
        header_panel = Panel(
            Align.center(header_text),
            title="[BUILD] Linux Build System",
            title_align="left",
            border_style="blue",
            padding=(1, 2)
        )
        
        self.console.print(header_panel)
        self.console.print()
        
        # Show build information
        info_table = Table(show_header=False, box=None, padding=(0, 2))
        info_table.add_column("Label", style="cyan", no_wrap=True)
        info_table.add_column("Value", style="white")
        
        info_table.add_row("[DIR] Project Directory:", str(self.project_root))
        info_table.add_row("[BUILD] Build Directory:", str(self.build_dir))
        info_table.add_row("[DEPLOY] Deploy Directory:", str(self.deploy_dir))
        info_table.add_row("[LOG] Log File:", str(self.log_file))
        info_table.add_row("[TIME] Started:", self.start_time.strftime("%Y-%m-%d %H:%M:%S"))
        info_table.add_row("[OS] Target Platform:", "Linux (Ubuntu/Debian)")
        
        self.console.print(Panel(info_table, title="[CONFIG] Build Configuration", border_style="green"))
        self.console.print()
    
    def run_command(self, command: List[str], description: str, cwd: Optional[Path] = None, 
                   timeout: int = 300) -> Tuple[bool, str, str]:
        """
        Run a command with rich output and logging.
        
        Returns:
            Tuple of (success, stdout, stderr)
        """
        cmd_str = " ".join(command)
        self.logger.info(f"Running command: {cmd_str}")
        self.logger.info(f"Working directory: {cwd or self.project_root}")
        
        # Show what we're running
        self.console.print(f"[RUNNING] {description}", style="bold blue")
        
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
                self.console.print(f"[OK] {description}", style="green")
                self.logger.info(f"Command succeeded: {cmd_str}")
            else:
                self.console.print(f"[FAIL] {description}", style="red")
                self.logger.error(f"Command failed: {cmd_str} (exit code: {result.returncode})")
                self.stats["total_errors"] += 1
            
            return success, stdout, stderr
            
        except subprocess.TimeoutExpired:
            self.console.print(f"[TIMEOUT] {description} - TIMEOUT", style="red")
            self.logger.error(f"Command timed out: {cmd_str}")
            self.stats["total_errors"] += 1
            return False, "", "Command timed out"
            
        except Exception as e:
            self.console.print(f"[ERROR] {description} - EXCEPTION", style="red")
            self.logger.error(f"Command exception: {cmd_str} - {str(e)}")
            self.stats["total_errors"] += 1
            return False, "", str(e)
    
    def verify_prerequisites(self) -> bool:
        """Verify all prerequisites are met before building."""
        self.console.print(Panel("[CHECK] Verifying Prerequisites", border_style="yellow"))
        
        checks = []
        
        # Check CMakeLists.txt
        cmake_file = self.project_root / "CMakeLists.txt"
        if cmake_file.exists():
            checks.append(("CMakeLists.txt", True, str(cmake_file)))
            self.logger.info(f"CMakeLists.txt found: {cmake_file}")
        else:
            checks.append(("CMakeLists.txt", False, f"Not found at {cmake_file}"))
            self.logger.error(f"CMakeLists.txt not found: {cmake_file}")
        
        # Check for Build Tools
        success, stdout, stderr = self.run_command(
            ["cmake", "--version"], 
            "CMake availability check"
        )
        checks.append(("CMake", success, "Available" if success else "Not found in PATH"))
        
        # Check GCC/Clang
        success, stdout, stderr = self.run_command(
            ["g++", "--version"], 
            "GCC availability check"
        )
        checks.append(("GCC Compiler", success, "Available" if success else "Not found in PATH"))
        
        # Display results
        table = Table(show_header=True, header_style="bold magenta")
        table.add_column("Prerequisite", style="cyan")
        table.add_column("Status", justify="center")
        table.add_column("Details", style="dim")
        
        all_good = True
        for name, status, details in checks:
            if status:
                table.add_row(name, "[OK] OK", details)
            else:
                table.add_row(name, "[FAIL] FAIL", details)
                all_good = False
        
        self.console.print(table)
        self.console.print()
        
        if not all_good:
            self.logger.error("Prerequisites check failed")
            self.console.print(Panel(
                "[red][FAIL] Prerequisites check failed![/red]\n\n"
                "Please ensure:\n"
                "• Qt6 is installed and in your PATH\n"
                "• CMake is available in PATH\n"
                "• GCC/Clang compiler is installed\n"
                "• CMakeLists.txt exists in project root",
                title="[FAIL] Build Cannot Continue",
                border_style="red"
            ))
            return False
        
        self.logger.info("All prerequisites verified successfully")
        return True
    
    def install_system_packages(self) -> bool:
        """Install required system packages for Qt6 development with rich progress."""
        self.console.print(Panel("[PACKAGES] Installing System Packages", border_style="yellow"))
        
        # Update package list first
        success, stdout, stderr = self.run_command(
            ["sudo", "apt", "update"], 
            "Updating package lists",
            timeout=300
        )
        if not success:
            self.console.print("[FAIL] Failed to update package lists", style="red")
            return False

        # Essential packages for Qt6 development on Linux
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
                        self.console.print(f"[FAIL] Failed to install package chunk: {chunk}", style="red")
                        return False
                    
                    progress.advance(install_task)

        except Exception as e:
            self.console.print(f"[FAIL] Exception during package installation: {e}", style="red")
            self.logger.error(f"Exception during package installation: {e}")
            return False

        self.stats["packages_installed"] = len(packages)
        self.console.print(f"[OK] Successfully installed {len(packages)} packages", style="bold green")
        self.logger.info(f"Successfully installed {len(packages)} packages")
        return True

    def verify_qt6_installation(self) -> bool:
        """Verify Qt6 installation with rich output."""
        self.console.print(Panel("[QT6] Verifying Qt6 Installation", border_style="cyan"))
        
        # Check qmake6
        success, stdout, stderr = self.run_command(
            ["/usr/lib/qt6/bin/qmake6", "-version"], 
            "Checking qmake6 version"
        )
        if not success:
            self.console.print("[FAIL] qmake6 not found or not working", style="red")
            return False
        
        # Extract Qt version
        if "Qt version" in stdout:
            qt_version_line = [line for line in stdout.split('\n') if 'Qt version' in line][0]
            self.stats["qt6_version"] = qt_version_line.split()[-1] if qt_version_line.split() else "Unknown"
        
        # Check for Qt6 CMake modules
        qt6_cmake_dir = Path("/usr/lib/x86_64-linux-gnu/cmake/Qt6")
        cmake_status = qt6_cmake_dir.exists()
        
        # Display Qt6 verification table
        qt_table = Table(show_header=True, header_style="bold cyan")
        qt_table.add_column("Component", style="cyan")
        qt_table.add_column("Status", justify="center")
        qt_table.add_column("Details", style="dim")
        
        qt_table.add_row("qmake6", "[OK] Found" if success else "[FAIL] Missing", 
                        f"Version: {self.stats['qt6_version']}" if success else "Not available")
        qt_table.add_row("CMake Modules", "[OK] Found" if cmake_status else "[FAIL] Missing", 
                        str(qt6_cmake_dir) if cmake_status else "Not found")
        
        self.console.print(qt_table)
        self.console.print()
        
        if success and cmake_status:
            self.console.print(f"[OK] Qt6 {self.stats['qt6_version']} verified successfully", style="bold green")
            self.logger.info(f"Qt6 {self.stats['qt6_version']} installation verified")
            return True
        else:
            self.console.print("[FAIL] Qt6 verification failed", style="bold red")
            self.logger.error("Qt6 verification failed")
            return False

    def clean_previous_builds(self) -> bool:
        """Clean previous build and deployment directories with rich output."""
        self.console.print(Panel("[CLEAN] Cleaning Previous Builds", border_style="yellow"))
        
        dirs_to_clean = [self.build_dir, self.deploy_dir]
        
        for dir_path in dirs_to_clean:
            if dir_path.exists():
                try:
                    self.console.print(f"[DELETE] Removing {dir_path.name}/", style="yellow")
                    shutil.rmtree(dir_path)
                    self.logger.info(f"Cleaned directory: {dir_path}")
                except Exception as e:
                    self.console.print(f"[WARN] Failed to remove {dir_path.name}/: {e}", style="red")
                    self.logger.warning(f"Failed to clean {dir_path}: {e}")
                    self.stats["total_warnings"] += 1
            else:
                self.console.print(f"[INFO] {dir_path.name}/ doesn't exist", style="dim")
        
        # Create build directory
        try:
            self.build_dir.mkdir(parents=True, exist_ok=True)
            self.console.print(f"[CREATE] Created {self.build_dir.name}/", style="green")
            self.logger.info(f"Created build directory: {self.build_dir}")
        except Exception as e:
            self.console.print(f"[FAIL] Failed to create build directory: {e}", style="red")
            self.logger.error(f"Failed to create build directory: {e}")
            return False
        
        self.console.print()
        return True

    def configure_cmake(self) -> bool:
        """Configure the project with CMake using rich progress."""
        self.console.print(Panel("[CMAKE] Configuring Project", border_style="blue"))
        
        # Try different Qt6 paths
        qt6_paths = [
            "/usr/lib/qt6",
            "/usr/lib/x86_64-linux-gnu/cmake/Qt6",
            "/usr/share/qt6"
        ]
        
        qt6_path = "/usr/lib/qt6"  # Default
        for path in qt6_paths:
            if Path(path).exists():
                qt6_path = path
                self.console.print(f"[FOUND] Using Qt6 path: {qt6_path}", style="cyan")
                break
        
        cmake_args = [
            "cmake",
            "-G", "Unix Makefiles",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            f"-DCMAKE_PREFIX_PATH={qt6_path}",
            "-DCMAKE_VERBOSE_MAKEFILE=ON",
            str(self.project_root)
        ]
        
        success, stdout, stderr = self.run_command(
            cmake_args, 
            "CMake configuration",
            cwd=self.build_dir, 
            timeout=600
        )
        
        if success:
            self.stats["cmake_successful"] = True
            self.console.print("[OK] CMake configuration successful", style="bold green")
            return True
        else:
            self.console.print(Panel(
                f"[red]CMake configuration failed![/red]\n\n"
                f"Error output:\n{stderr}\n\n"
                "Trying with Ninja generator as fallback...",
                title="[WARN] CMake Configuration Error",
                border_style="yellow"
            ))
            
            # Try with Ninja generator as fallback
            cmake_args[2] = "Ninja"
            success, stdout, stderr = self.run_command(
                cmake_args, 
                "CMake configuration (Ninja fallback)",
                cwd=self.build_dir, 
                timeout=600
            )
            
            if success:
                self.stats["cmake_successful"] = True
                self.console.print("[OK] CMake configuration successful with Ninja", style="bold green")
                return True
            else:
                self.console.print(Panel(
                    f"[red]CMake configuration failed with both generators![/red]\n\n"
                    f"Error output:\n{stderr}",
                    title="[FAIL] CMake Configuration Error",
                    border_style="red"
                ))
                return False

    def build_plugins(self) -> bool:
        """Build all plugins with rich progress tracking."""
        self.console.print(Panel("[PLUGINS] Building Plugins", border_style="blue"))
        
        try:
            with Progress(
                SpinnerColumn(),
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                TaskProgressColumn(),
                console=self.console
            ) as progress:
                
                plugin_task = progress.add_task("Building plugins...", total=len(self.plugins))
                
                for plugin in self.plugins:
                    progress.update(plugin_task, description=f"Building {plugin}...")
                    
                    success, stdout, stderr = self.run_command(
                        ["cmake", "--build", ".", "--target", plugin, "--config", self.build_config, "--parallel", str(multiprocessing.cpu_count())],
                        f"Building {plugin}",
                        cwd=self.build_dir,
                        timeout=900  # 15 minute timeout
                    )
                    
                    if success:
                        self.stats["plugins_built"] += 1
                        self.console.print(f"[OK] {plugin} built successfully", style="green")
                    else:
                        self.stats["plugins_failed"] += 1
                        self.console.print(f"[FAIL] {plugin} build failed", style="red")
                        self.console.print(Panel(
                            f"[red]Plugin build failed: {plugin}[/red]\n\n"
                            f"Error output:\n{stderr}",
                            title=f"[FAIL] {plugin} Build Error",
                            border_style="red"
                        ))
                        return False
                    
                    progress.advance(plugin_task)
                    
        except Exception as e:
            self.console.print(f"[FAIL] Error during plugin building: {e}", style="red")
            self.logger.error(f"Exception during plugin building: {e}")
            return False
        
        return True

    def build_main_application(self) -> bool:
        """Build the main application with rich output."""
        self.console.print(Panel("[MAIN] Building Main Application", border_style="blue"))
        
        success, stdout, stderr = self.run_command(
            ["cmake", "--build", ".", "--target", "ItemEditor", "--config", self.build_config, "--parallel", str(multiprocessing.cpu_count())],
            "Building ItemEditor executable",
            cwd=self.build_dir,
            timeout=1200  # 20 minute timeout
        )
        
        if success:
            self.stats["main_app_built"] = True
            
            # Find the executable
            possible_paths = [
                self.build_dir / "bin" / "ItemEditor",
                self.build_dir / "src" / "ItemEditor",
                self.build_dir / "ItemEditor"
            ]
            
            exe_path = None
            for path in possible_paths:
                if path.exists():
                    exe_path = path
                    break
            
            if exe_path:
                file_size = exe_path.stat().st_size
                self.console.print(f"[OK] ItemEditor built successfully ({file_size:,} bytes)", style="bold green")
                self.logger.info(f"Main application built: {exe_path} ({file_size:,} bytes)")
                return True
            else:
                self.console.print("[WARN] ItemEditor built but executable not found in expected locations", style="yellow")
                return True
        else:
            self.console.print(Panel(
                f"[red]ItemEditor build failed![/red]\n\n"
                f"Error output:\n{stderr}",
                title="[FAIL] Main Application Build Error",
                border_style="red"
            ))
            return False

    def show_build_summary(self):
        """Display a comprehensive build summary with rich formatting."""
        end_time = datetime.now()
        duration = end_time - self.start_time
        
        # Create summary panel
        if self.stats["build_successful"]:
            summary_style = "bold green"
            summary_text = "[SUCCESS] BUILD COMPLETED SUCCESSFULLY!"
            border_style = "green"
        else:
            summary_style = "bold red"
            summary_text = "[FAIL] BUILD FAILED"
            border_style = "red"
        
        # Build statistics table
        stats_table = Table(show_header=False, box=None, padding=(0, 2))
        stats_table.add_column("Metric", style="cyan", no_wrap=True)
        stats_table.add_column("Value", style="white")
        
        stats_table.add_row("[TIME] Duration:", f"{duration.total_seconds():.1f} seconds")
        stats_table.add_row("[QT6] Qt6 Version:", self.stats["qt6_version"])
        stats_table.add_row("[PKG] Packages Installed:", str(self.stats["packages_installed"]))
        stats_table.add_row("[CMAKE] CMake Config:", "[OK] Success" if self.stats["cmake_successful"] else "[FAIL] Failed")
        stats_table.add_row("[PLUGINS] Plugins Built:", f"{self.stats['plugins_built']}/{len(self.plugins)}")
        stats_table.add_row("[MAIN] Main App:", "[OK] Success" if self.stats["main_app_built"] else "[FAIL] Failed")
        stats_table.add_row("[WARN] Warnings:", str(self.stats["total_warnings"]))
        stats_table.add_row("[ERROR] Errors:", str(self.stats["total_errors"]))
        
        self.console.print()
        self.console.print(Panel(
            Text(summary_text, style=summary_style),
            title="[SUMMARY] Build Summary",
            border_style=border_style,
            padding=(1, 2)
        ))
        
        self.console.print(Panel(stats_table, title="[STATS] Build Statistics", border_style=border_style))
        
        if self.stats["build_successful"]:
            # Show next steps
            next_steps = Text()
            next_steps.append("[LOCATION] Build Location:\n", style="bold blue")
            next_steps.append(f"   {self.build_dir}\n\n", style="white")
            next_steps.append("[RUN] To run the application:\n", style="bold blue")
            next_steps.append("   1. Navigate to the build directory\n", style="white")
            next_steps.append("   2. Find the ItemEditor executable\n", style="white")
            next_steps.append("   3. Run: ./ItemEditor\n\n", style="white")
            next_steps.append("[LOG] Build log saved to:\n", style="bold blue")
            next_steps.append(f"   {self.log_file}\n", style="white")
            
            self.console.print(Panel(next_steps, title="[NEXT] Next Steps", border_style="blue"))
        
        # Log final summary
        self.logger.info(f"Build completed in {duration.total_seconds():.1f} seconds")
        self.logger.info(f"Final status: {'SUCCESS' if self.stats['build_successful'] else 'FAILED'}")
        self.logger.info(f"Log file saved to: {self.log_file}")

    def run_build(self) -> bool:
        """Run the complete build process with rich UI."""
        try:
            self.show_header()
            
            # Step 1: Prerequisites
            if not self.verify_prerequisites():
                return False
            
            # Step 2: Install packages
            if not self.install_system_packages():
                return False
            
            # Step 3: Verify Qt6
            if not self.verify_qt6_installation():
                return False
            
            # Step 4: Clean previous builds
            if not self.clean_previous_builds():
                return False
            
            # Step 5: Configure CMake
            if not self.configure_cmake():
                return False
            
            # Step 6: Build plugins
            if not self.build_plugins():
                return False
            
            # Step 7: Build main application
            if not self.build_main_application():
                return False
            
            self.stats["build_successful"] = True
            return True
            
        except KeyboardInterrupt:
            self.console.print("\n[STOP] Build interrupted by user", style="bold red")
            self.logger.warning("Build interrupted by user")
            return False
        except Exception as e:
            self.console.print(f"\n[ERROR] Unexpected error: {e}", style="bold red")
            self.logger.error(f"Unexpected error: {e}")
            return False
        finally:
            self.show_build_summary()


def main():
    """Main entry point with rich error handling."""
    console = Console()
    
    # Check Python version
    if sys.version_info < (3, 7):
        console.print("[FAIL] Python 3.7 or higher is required", style="bold red")
        sys.exit(1)
    
    # Check if running on Linux
    if sys.platform != "linux":
        console.print("[WARN] This script is optimized for Linux", style="bold yellow")
        if not Confirm.ask("Continue anyway?"):
            sys.exit(1)
    
    # Create and run builder
    builder = LinuxBuilder()
    success = builder.run_build()
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()