#!/usr/bin/env python3
"""
Item Editor Qt6 - Linux Ubuntu Build Script
==========================================

This script sets up the complete build environment for Ubuntu Linux including:
- Python dependencies installation
- System package dependencies (CMake, Qt6, compilers)
- Environment preparation
- Qt6 project compilation
- Plugin building and deployment

Tested on: Ubuntu 20.04, 22.04, 24.04
Requirements: Python 3.8+, sudo access for package installation

Author: Generated for Item Editor Qt6 Project
"""

import os
import sys
import subprocess
import shutil
import logging
import time
import platform
from datetime import datetime
from pathlib import Path
from typing import Optional, List, Tuple, Dict

# First, install Python dependencies if needed
def install_python_dependencies():
    """Install required Python packages."""
    required_packages = ['rich>=13.0.0']

    print("Checking Python dependencies...")

    for package in required_packages:
        try:
            package_name = package.split('>=')[0]
            __import__(package_name)
            print(f"✓ {package_name} is already installed")
        except ImportError:
            print(f"Installing {package}...")
            try:
                subprocess.check_call([sys.executable, '-m', 'pip', 'install', package])
                print(f"✓ Successfully installed {package}")
            except subprocess.CalledProcessError as e:
                print(f"✗ Failed to install {package}: {e}")
                print("Trying with --user flag...")
                try:
                    subprocess.check_call([sys.executable, '-m', 'pip', 'install', '--user', package])
                    print(f"✓ Successfully installed {package} with --user")
                except subprocess.CalledProcessError as e2:
                    print(f"✗ Failed to install {package} even with --user: {e2}")
                    print("Please install manually: pip install rich")
                    return False
    return True

# Install Python dependencies first
if not install_python_dependencies():
    sys.exit(1)

# Now import rich after ensuring it's installed
try:
    from rich.console import Console
    from rich.panel import Panel
    from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn, TaskProgressColumn
    from rich.table import Table
    from rich.text import Text
    from rich.logging import RichHandler
    from rich.prompt import Confirm, Prompt
    from rich.tree import Tree
    from rich.layout import Layout
    from rich.live import Live
    from rich.align import Align
    from rich.markup import escape
except ImportError as e:
    print(f"ERROR: Failed to import rich library: {e}")
    print("Please install it manually with: pip install rich")
    sys.exit(1)


class LinuxQt6Builder:
    """Main builder class for Qt6 Item Editor project on Linux Ubuntu."""

    def __init__(self):
        self.console = Console(force_terminal=True, width=120)
        self.start_time = datetime.now()

        # System information
        self.distro_info = self.get_distro_info()
        self.python_version = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"

        # Project paths
        self.project_dir = Path(__file__).parent.absolute()
        self.build_dir = self.project_dir / "build"
        self.deploy_dir = self.project_dir / "deploy_linux"

        # Build configuration
        self.plugins = ["PluginOne", "PluginTwo", "PluginThree"]
        self.cmake_generator = "Unix Makefiles"
        self.build_config = "Release"

        # System packages required
        self.required_packages = [
            "build-essential",
            "cmake",
            "git",
            "pkg-config",
            "qt6-base-dev",
            "qt6-tools-dev",
            "qt6-tools-dev-tools",
            "libqt6core6",
            "libqt6gui6",
            "libqt6widgets6",
            "libqt6network6",
            "libqt6xml6",
            "qt6-qpa-plugins",
            "libgl1-mesa-dev",
            "libglu1-mesa-dev",
            "libxkbcommon-dev",
            "libxcb-xinerama0-dev",
            "libxcb-cursor-dev",
            "libfontconfig1-dev",
            "libfreetype6-dev",
            "libx11-dev",
            "libxext-dev",
            "libxfixes-dev",
            "libxi-dev",
            "libxrender-dev",
            "libxcb1-dev",
            "libx11-xcb-dev",
            "libxcb-glx0-dev",
            "libxcb-keysyms1-dev",
            "libxcb-image0-dev",
            "libxcb-shm0-dev",
            "libxcb-icccm4-dev",
            "libxcb-sync-dev",
            "libxcb-xfixes0-dev",
            "libxcb-shape0-dev",
            "libxcb-randr0-dev",
            "libxcb-render-util0-dev"
        ]

        # Setup logging
        self.setup_logging()

        # Build statistics
        self.stats = {
            "system_packages_installed": 0,
            "plugins_built": 0,
            "plugins_failed": 0,
            "main_app_built": False,
            "deployment_successful": False,
            "total_errors": 0,
            "total_warnings": 0,
            "qt6_version": "Unknown"
        }

    def get_distro_info(self) -> Dict[str, str]:
        """Get Linux distribution information."""
        info = {
            "name": "Unknown",
            "version": "Unknown",
            "codename": "Unknown"
        }

        try:
            # Try to read /etc/os-release
            if Path("/etc/os-release").exists():
                with open("/etc/os-release", "r") as f:
                    for line in f:
                        if line.startswith("NAME="):
                            info["name"] = line.split("=")[1].strip().strip('"')
                        elif line.startswith("VERSION="):
                            info["version"] = line.split("=")[1].strip().strip('"')
                        elif line.startswith("VERSION_CODENAME="):
                            info["codename"] = line.split("=")[1].strip().strip('"')
        except Exception:
            pass

        return info

    def setup_logging(self):
        """Setup comprehensive logging to file and console."""
        # Create logs directory
        log_dir = self.project_dir / "logs"
        log_dir.mkdir(exist_ok=True)

        # Create log filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = log_dir / f"build_linux_{timestamp}.log"

        # Setup file logging
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

    def print_header(self):
        """Print a beautiful header with system information."""
        header_text = f"""
[bold blue]Item Editor Qt6 - Linux Ubuntu Build Script[/bold blue]
[dim]Automated build environment setup and compilation[/dim]

[bold]System Information:[/bold]
• Distribution: {self.distro_info['name']} {self.distro_info['version']} ({self.distro_info['codename']})
• Architecture: {platform.machine()}
• Python: {self.python_version}
• Kernel: {platform.release()}
• Build Time: {self.start_time.strftime('%Y-%m-%d %H:%M:%S')}

[bold]Project Information:[/bold]
• Project Directory: {self.project_dir}
• Build Directory: {self.build_dir}
• Deploy Directory: {self.deploy_dir}
• CMake Generator: {self.cmake_generator}
• Build Configuration: {self.build_config}
"""

        panel = Panel(
            header_text,
            title="🚀 Qt6 Linux Build Environment",
            border_style="blue",
            padding=(1, 2)
        )

        self.console.print(panel)
        self.console.print()

    def check_sudo_access(self) -> bool:
        """Check if user has sudo access."""
        try:
            result = subprocess.run(['sudo', '-n', 'true'],
                                  capture_output=True,
                                  timeout=5)
            return result.returncode == 0
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError):
            return False

    def run_command(self, cmd: List[str], cwd: Optional[Path] = None,
                   capture_output: bool = False, timeout: int = 300) -> Tuple[bool, str, str]:
        """Run a command with proper error handling and logging."""
        cmd_str = ' '.join(cmd)
        self.logger.info(f"Running command: {cmd_str}")

        if cwd:
            self.logger.info(f"Working directory: {cwd}")

        try:
            if capture_output:
                result = subprocess.run(
                    cmd,
                    cwd=cwd,
                    capture_output=True,
                    text=True,
                    timeout=timeout,
                    encoding='utf-8',
                    errors='replace'
                )

                stdout = result.stdout.strip()
                stderr = result.stderr.strip()

                if result.returncode == 0:
                    self.logger.info(f"Command succeeded: {cmd_str}")
                    if stdout:
                        self.logger.debug(f"STDOUT: {stdout}")
                else:
                    self.logger.error(f"Command failed with code {result.returncode}: {cmd_str}")
                    if stderr:
                        self.logger.error(f"STDERR: {stderr}")
                    if stdout:
                        self.logger.debug(f"STDOUT: {stdout}")

                return result.returncode == 0, stdout, stderr
            else:
                result = subprocess.run(cmd, cwd=cwd, timeout=timeout)
                success = result.returncode == 0

                if success:
                    self.logger.info(f"Command succeeded: {cmd_str}")
                else:
                    self.logger.error(f"Command failed with code {result.returncode}: {cmd_str}")

                return success, "", ""

        except subprocess.TimeoutExpired:
            self.logger.error(f"Command timed out after {timeout}s: {cmd_str}")
            return False, "", f"Command timed out after {timeout} seconds"
        except Exception as e:
            self.logger.error(f"Command execution failed: {cmd_str} - {str(e)}")
            return False, "", str(e)

    def update_package_lists(self) -> bool:
        """Update apt package lists."""
        self.console.print("[bold yellow]📦 Updating package lists...[/bold yellow]")

        success, stdout, stderr = self.run_command(['sudo', 'apt', 'update'], timeout=120)

        if success:
            self.console.print("[green]✓ Package lists updated successfully[/green]")
            return True
        else:
            self.console.print(f"[red]✗ Failed to update package lists: {stderr}[/red]")
            return False

    def install_system_packages(self) -> bool:
        """Install required system packages."""
        self.console.print("[bold yellow]📦 Installing system packages...[/bold yellow]")

        # Check which packages are already installed
        installed_packages = []
        missing_packages = []

        with Progress(
            SpinnerColumn(),
            TextColumn("[progress.description]{task.description}"),
            BarColumn(),
            TaskProgressColumn(),
            console=self.console
        ) as progress:

            check_task = progress.add_task("Checking installed packages...", total=len(self.required_packages))

            for package in self.required_packages:
                progress.update(check_task, description=f"Checking {package}...")

                success, stdout, stderr = self.run_command(
                    ['dpkg', '-l', package],
                    capture_output=True
                )

                if success and 'ii' in stdout:
                    installed_packages.append(package)
                else:
                    missing_packages.append(package)

                progress.advance(check_task)

        self.console.print(f"[green]✓ {len(installed_packages)} packages already installed[/green]")

        if missing_packages:
            self.console.print(f"[yellow]⚠ {len(missing_packages)} packages need to be installed[/yellow]")

            # Install missing packages
            install_cmd = ['sudo', 'apt', 'install', '-y'] + missing_packages

            self.console.print("[bold blue]Installing missing packages...[/bold blue]")
            success, stdout, stderr = self.run_command(install_cmd, timeout=600)

            if success:
                self.console.print("[green]✓ All system packages installed successfully[/green]")
                self.stats["system_packages_installed"] = len(missing_packages)
                return True
            else:
                self.console.print(f"[red]✗ Failed to install packages: {stderr}[/red]")
                return False
        else:
            self.console.print("[green]✓ All required packages are already installed[/green]")
            return True

    def detect_qt6_installation(self) -> Optional[Path]:
        """Detect Qt6 installation path."""
        self.console.print("[bold yellow]🔍 Detecting Qt6 installation...[/bold yellow]")

        # Common Qt6 installation paths on Ubuntu
        possible_paths = [
            Path("/usr/lib/qt6"),
            Path("/usr/lib/x86_64-linux-gnu/qt6"),
            Path("/opt/qt6"),
            Path("/usr/local/qt6"),
            Path.home() / "Qt" / "6.5.0" / "gcc_64",
            Path.home() / "Qt" / "6.6.0" / "gcc_64",
            Path.home() / "Qt" / "6.7.0" / "gcc_64",
            Path.home() / "Qt" / "6.8.0" / "gcc_64",
        ]

        # Try to find qmake6 or qmake
        for qt_path in possible_paths:
            qmake_path = qt_path / "bin" / "qmake6"
            if not qmake_path.exists():
                qmake_path = qt_path / "bin" / "qmake"

            if qmake_path.exists():
                # Verify it's Qt6
                success, stdout, stderr = self.run_command(
                    [str(qmake_path), '-version'],
                    capture_output=True
                )

                if success and 'Qt version 6' in stdout:
                    self.console.print(f"[green]✓ Found Qt6 at: {qt_path}[/green]")
                    # Extract Qt version
                    for line in stdout.split('\n'):
                        if 'Qt version' in line:
                            version = line.split('Qt version')[1].strip()
                            self.stats["qt6_version"] = version
                            self.console.print(f"[blue]  Version: {version}[/blue]")
                            break
                    return qt_path

        # Try system-wide qmake6
        success, stdout, stderr = self.run_command(['which', 'qmake6'], capture_output=True)
        if success:
            qmake_path = Path(stdout.strip())
            qt_path = qmake_path.parent.parent
            self.console.print(f"[green]✓ Found system Qt6 at: {qt_path}[/green]")

            # Get version
            success, stdout, stderr = self.run_command(['qmake6', '-version'], capture_output=True)
            if success and 'Qt version 6' in stdout:
                for line in stdout.split('\n'):
                    if 'Qt version' in line:
                        version = line.split('Qt version')[1].strip()
                        self.stats["qt6_version"] = version
                        self.console.print(f"[blue]  Version: {version}[/blue]")
                        break
            return qt_path

        self.console.print("[red]✗ Qt6 installation not found[/red]")
        return None

    def setup_build_environment(self) -> bool:
        """Setup build environment variables."""
        self.console.print("[bold yellow]🔧 Setting up build environment...[/bold yellow]")

        # Detect Qt6
        qt_path = self.detect_qt6_installation()
        if not qt_path:
            self.console.print("[red]✗ Qt6 not found. Please install Qt6 development packages.[/red]")
            return False

        # Set environment variables
        os.environ['Qt6_DIR'] = str(qt_path)
        os.environ['CMAKE_PREFIX_PATH'] = str(qt_path)

        # Add Qt6 bin to PATH if not already there
        qt_bin = qt_path / "bin"
        if qt_bin.exists():
            current_path = os.environ.get('PATH', '')
            if str(qt_bin) not in current_path:
                os.environ['PATH'] = f"{qt_bin}:{current_path}"

        self.console.print("[green]✓ Build environment configured[/green]")
        return True

    def clean_build_directory(self) -> bool:
        """Clean the build directory."""
        self.console.print("[bold yellow]🧹 Cleaning build directory...[/bold yellow]")

        if self.build_dir.exists():
            try:
                shutil.rmtree(self.build_dir)
                self.console.print("[green]✓ Build directory cleaned[/green]")
            except Exception as e:
                self.console.print(f"[red]✗ Failed to clean build directory: {e}[/red]")
                return False

        # Create fresh build directory
        self.build_dir.mkdir(parents=True, exist_ok=True)
        return True

    def configure_cmake(self) -> bool:
        """Configure the project with CMake."""
        self.console.print("[bold yellow]⚙️ Configuring CMake...[/bold yellow]")

        cmake_args = [
            'cmake',
            '-G', self.cmake_generator,
            f'-DCMAKE_BUILD_TYPE={self.build_config}',
            '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
            str(self.project_dir)
        ]

        # Add Qt6 path if detected
        if 'CMAKE_PREFIX_PATH' in os.environ:
            cmake_args.append(f"-DCMAKE_PREFIX_PATH={os.environ['CMAKE_PREFIX_PATH']}")

        success, stdout, stderr = self.run_command(cmake_args, cwd=self.build_dir, timeout=120)

        if success:
            self.console.print("[green]✓ CMake configuration successful[/green]")
            return True
        else:
            self.console.print(f"[red]✗ CMake configuration failed: {stderr}[/red]")
            self.stats["total_errors"] += 1
            return False

    def build_project(self) -> bool:
        """Build the main project."""
        self.console.print("[bold yellow]🔨 Building main application...[/bold yellow]")

        # Determine number of parallel jobs
        import multiprocessing
        num_jobs = multiprocessing.cpu_count()

        build_args = [
            'cmake',
            '--build', '.',
            '--config', self.build_config,
            '--parallel', str(num_jobs)
        ]

        success, stdout, stderr = self.run_command(build_args, cwd=self.build_dir, timeout=600)

        if success:
            self.console.print("[green]✓ Main application built successfully[/green]")
            self.stats["main_app_built"] = True
            return True
        else:
            self.console.print(f"[red]✗ Main application build failed: {stderr}[/red]")
            self.stats["total_errors"] += 1
            return False

    def verify_build_artifacts(self) -> bool:
        """Verify that build artifacts were created."""
        self.console.print("[bold yellow]🔍 Verifying build artifacts...[/bold yellow]")

        # Check for main executable
        main_exe = self.build_dir / "src" / "ItemEditor"
        if not main_exe.exists():
            self.console.print(f"[red]✗ Main executable not found: {main_exe}[/red]")
            return False

        self.console.print(f"[green]✓ Main executable found: {main_exe}[/green]")

        # Check for plugins
        plugin_dir = self.build_dir / "plugins"
        plugins_found = 0

        for plugin_name in self.plugins:
            plugin_lib = plugin_dir / plugin_name / f"lib{plugin_name}.so"
            if plugin_lib.exists():
                self.console.print(f"[green]✓ Plugin found: {plugin_lib}[/green]")
                plugins_found += 1
            else:
                self.console.print(f"[yellow]⚠ Plugin not found: {plugin_lib}[/yellow]")

        self.stats["plugins_built"] = plugins_found
        self.stats["plugins_failed"] = len(self.plugins) - plugins_found

        return True

    def create_deployment_package(self) -> bool:
        """Create deployment package."""
        self.console.print("[bold yellow]📦 Creating deployment package...[/bold yellow]")

        # Clean deployment directory
        if self.deploy_dir.exists():
            shutil.rmtree(self.deploy_dir)

        self.deploy_dir.mkdir(parents=True, exist_ok=True)

        try:
            # Copy main executable
            main_exe = self.build_dir / "src" / "ItemEditor"
            if main_exe.exists():
                shutil.copy2(main_exe, self.deploy_dir / "ItemEditor")
                os.chmod(self.deploy_dir / "ItemEditor", 0o755)
                self.console.print("[green]✓ Main executable copied[/green]")

            # Copy plugins
            plugin_deploy_dir = self.deploy_dir / "plugins"
            plugin_deploy_dir.mkdir(exist_ok=True)

            plugin_dir = self.build_dir / "plugins"
            for plugin_name in self.plugins:
                plugin_lib = plugin_dir / plugin_name / f"lib{plugin_name}.so"
                if plugin_lib.exists():
                    shutil.copy2(plugin_lib, plugin_deploy_dir / f"lib{plugin_name}.so")
                    self.console.print(f"[green]✓ Plugin {plugin_name} copied[/green]")

            # Create run script
            run_script = self.deploy_dir / "run_itemeditor.sh"
            with open(run_script, 'w') as f:
                f.write("""#!/bin/bash
# Item Editor Qt6 - Linux Run Script

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set library path to include Qt6 libraries
export LD_LIBRARY_PATH="/usr/lib/qt6/lib:/usr/lib/x86_64-linux-gnu/qt6/lib:$LD_LIBRARY_PATH"

# Set plugin path
export QT_PLUGIN_PATH="$SCRIPT_DIR/plugins:/usr/lib/qt6/plugins:/usr/lib/x86_64-linux-gnu/qt6/plugins"

# Run the application
cd "$SCRIPT_DIR"
./ItemEditor "$@"
""")
            os.chmod(run_script, 0o755)

            self.console.print("[green]✓ Deployment package created successfully[/green]")
            self.stats["deployment_successful"] = True
            return True

        except Exception as e:
            self.console.print(f"[red]✗ Failed to create deployment package: {e}[/red]")
            return False

    def print_build_summary(self):
        """Print a comprehensive build summary."""
        duration = datetime.now() - self.start_time

        # Create summary table
        table = Table(title="🎯 Build Summary", show_header=True, header_style="bold blue")
        table.add_column("Component", style="cyan", no_wrap=True)
        table.add_column("Status", justify="center")
        table.add_column("Details", style="dim")

        # Main application
        main_status = "[green]✓ Success[/green]" if self.stats["main_app_built"] else "[red]✗ Failed[/red]"
        table.add_row("Main Application", main_status, "ItemEditor executable")

        # Plugins
        plugin_status = f"[green]✓ {self.stats['plugins_built']} Built[/green]"
        if self.stats["plugins_failed"] > 0:
            plugin_status += f" [red]✗ {self.stats['plugins_failed']} Failed[/red]"
        table.add_row("Plugins", plugin_status, f"Total: {len(self.plugins)}")

        # Deployment
        deploy_status = "[green]✓ Success[/green]" if self.stats["deployment_successful"] else "[red]✗ Failed[/red]"
        table.add_row("Deployment", deploy_status, str(self.deploy_dir))

        # System info
        table.add_row("Qt6 Version", f"[blue]{self.stats['qt6_version']}[/blue]", "Detected version")
        table.add_row("Build Time", f"[yellow]{duration}[/yellow]", "Total duration")
        table.add_row("Log File", f"[dim]{self.log_file}[/dim]", "Detailed logs")

        self.console.print()
        self.console.print(table)

        # Print next steps
        if self.stats["main_app_built"] and self.stats["deployment_successful"]:
            next_steps = f"""
[bold green]🎉 Build completed successfully![/bold green]

[bold]Next steps:[/bold]
1. Navigate to deployment directory: [cyan]cd {self.deploy_dir}[/cyan]
2. Run the application: [cyan]./run_itemeditor.sh[/cyan]
3. Or run directly: [cyan]./ItemEditor[/cyan]

[bold]Deployment contents:[/bold]
• ItemEditor - Main executable
• plugins/ - Plugin libraries
• run_itemeditor.sh - Launcher script with environment setup

[dim]Note: The run script sets up proper library paths for Qt6[/dim]
"""
        else:
            next_steps = f"""
[bold red]❌ Build failed![/bold red]

[bold]Troubleshooting:[/bold]
1. Check the log file: [cyan]{self.log_file}[/cyan]
2. Verify Qt6 installation: [cyan]qmake6 -version[/cyan]
3. Check system packages: [cyan]apt list --installed | grep qt6[/cyan]
4. Review CMake configuration errors above

[bold]Common issues:[/bold]
• Missing Qt6 development packages
• Insufficient disk space
• Network issues during package installation
"""

        panel = Panel(
            next_steps,
            border_style="green" if self.stats["main_app_built"] else "red",
            padding=(1, 2)
        )

        self.console.print(panel)

    def run_build_process(self) -> bool:
        """Run the complete build process."""
        try:
            self.print_header()

            # Check sudo access
            if not self.check_sudo_access():
                self.console.print("[red]✗ This script requires sudo access to install system packages.[/red]")
                self.console.print("[yellow]Please run: sudo -v[/yellow]")
                return False

            # Step 1: Update package lists
            if not self.update_package_lists():
                return False

            # Step 2: Install system packages
            if not self.install_system_packages():
                return False

            # Step 3: Setup build environment
            if not self.setup_build_environment():
                return False

            # Step 4: Clean build directory
            if not self.clean_build_directory():
                return False

            # Step 5: Configure CMake
            if not self.configure_cmake():
                return False

            # Step 6: Build project
            if not self.build_project():
                return False

            # Step 7: Verify build artifacts
            if not self.verify_build_artifacts():
                return False

            # Step 8: Create deployment package
            if not self.create_deployment_package():
                return False

            return True

        except KeyboardInterrupt:
            self.console.print("\n[red]Build interrupted by user[/red]")
            return False
        except Exception as e:
            self.console.print(f"\n[red]Unexpected error: {e}[/red]")
            self.logger.exception("Unexpected error during build process")
            return False


def main():
    """Main entry point."""
    builder = LinuxQt6Builder()

    try:
        success = builder.run_build_process()
        builder.print_build_summary()

        sys.exit(0 if success else 1)

    except Exception as e:
        builder.console.print(f"[red]Fatal error: {e}[/red]")
        builder.logger.exception("Fatal error in main")
        sys.exit(1)


if __name__ == "__main__":
    main()
