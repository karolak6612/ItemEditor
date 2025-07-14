#!/usr/bin/env python3
"""
Item Editor Qt6 - Python Build Script with Rich Graphics and Logging
==================================================================

This script replaces the batch file with a Python implementation featuring:
- Rich console output with colors and progress bars
- Comprehensive logging to file for debugging
- Better error handling and diagnostics
- Cross-platform compatibility
- Detailed build verification

Author: Generated for Item Editor Qt6 Project
"""

import os
import sys
import subprocess
import shutil
import logging
import time
from datetime import datetime
from pathlib import Path
from typing import Optional, List, Tuple

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
except ImportError:
    print("ERROR: 'rich' library not found!")
    print("Please install it with: pip install rich")
    print("This script requires the 'rich' library for enhanced console output.")
    sys.exit(1)


class Qt6Builder:
    """Main builder class for Qt6 Item Editor project."""
    
    def __init__(self):
        # Set up console with proper encoding for Windows
        try:
            # Try to enable UTF-8 mode for Windows console
            import locale
            locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')
        except:
            pass
        
        # Initialize console with fallback for Windows
        try:
            self.console = Console(force_terminal=True, legacy_windows=False)
        except:
            # Fallback for older Windows systems
            self.console = Console(force_terminal=True, legacy_windows=True)
        
        self.start_time = datetime.now()
        
        # Project paths
        self.project_dir = Path(__file__).parent.absolute()
        self.build_dir = self.project_dir / "build"
        self.deploy_dir = self.project_dir / "deploy_threestep"

        # Setup logging
        self.setup_logging()
        
        # Build configuration
        self.plugins = ["PluginOne", "PluginTwo", "PluginThree"]
        self.build_config = "Release"
        
        # Build statistics
        self.stats = {
            "plugins_built": 0,
            "plugins_failed": 0,
            "main_app_built": False,
            "deployment_successful": False,
            "total_errors": 0,
            "total_warnings": 0
        }
    
    def setup_logging(self):
        """Setup comprehensive logging to file and console."""
        # Create logs directory
        log_dir = self.project_dir / "logs"
        log_dir.mkdir(exist_ok=True)
        
        # Create log filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = log_dir / f"build_{timestamp}.log"
        
        # Configure logging
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
        
        self.logger.info(f"Build session started - Log file: {log_file}")
        self.logger.info(f"Project directory: {self.project_dir}")
    
    def show_header(self):
        """Display the application header with rich formatting."""
        header_text = Text()
        header_text.append("Item Editor Qt6", style="bold blue")
        header_text.append(" - ", style="white")
        header_text.append("Python Build Script", style="bold green")
        
        header_panel = Panel(
            Align.center(header_text),
            title="[BUILD] Build System",
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
        
        info_table.add_row("[DIR] Project Directory:", str(self.project_dir))
        info_table.add_row("[BUILD] Build Directory:", str(self.build_dir))
        info_table.add_row("[DEPLOY] Deploy Directory:", str(self.deploy_dir))
        info_table.add_row("[LOG] Log File:", str(self.log_file))
        info_table.add_row("[TIME] Started:", self.start_time.strftime("%Y-%m-%d %H:%M:%S"))
        
        self.console.print(Panel(info_table, title="[CONFIG] Build Configuration", border_style="green"))
        self.console.print()
    
    def run_command(self, command: List[str], description: str, cwd: Optional[Path] = None) -> Tuple[bool, str, str]:
        """
        Run a command with rich output and logging.
        
        Returns:
            Tuple of (success, stdout, stderr)
        """
        cmd_str = " ".join(command)
        self.logger.info(f"Running command: {cmd_str}")
        self.logger.info(f"Working directory: {cwd or self.project_dir}")
        
        # Show what we're running without using console.status to avoid Rich display conflicts
        self.console.print(f"[RUNNING] Running: {description}", style="bold blue")
        
        try:
            result = subprocess.run(
                command,
                cwd=cwd or self.project_dir,
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
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
        cmake_file = self.project_dir / "CMakeLists.txt"
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
                "• A compatible C++ compiler is installed\n"
                "• CMakeLists.txt exists in project root",
                title="[FAIL] Build Cannot Continue",
                border_style="red"
            ))
            return False
        
        self.logger.info("All prerequisites verified successfully")
        return True
    
    def clean_previous_builds(self) -> bool:
        """Clean previous build and deployment directories."""
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
    
    def step1_build_plugins(self) -> bool:
        """Step 1: Build all plugins."""
        self.console.print(Panel("[PLUGINS] STEP 1: Building Plugins", border_style="blue", title_align="left"))
        
        # Configure CMake
        self.console.print("[CMAKE] Configuring CMake...", style="bold blue")
        success, stdout, stderr = self.run_command(
            ["cmake", "..", f"-DCMAKE_BUILD_TYPE={self.build_config}"],
            "CMake configuration",
            cwd=self.build_dir
        )
        
        if not success:
            self.console.print(Panel(
                f"[red]CMake configuration failed![/red]\n\n"
                f"Error output:\n{stderr}\n\n"
                "Common solutions:\n"
                "• Ensure Visual Studio 2022 is installed\n"
                "• Check CMakeLists.txt syntax\n"
                "• Verify Qt6 installation",
                title="[FAIL] Configuration Error",
                border_style="red"
            ))
            return False
        
        # Build each plugin with progress tracking
        try:
            with Progress(
                SpinnerColumn(),
                TextColumn("[progress.description]{task.description}"),
                BarColumn(),
                TaskProgressColumn(),
                console=self.console
            ) as progress:
                
                plugin_task = progress.add_task("Building plugins...", total=len(self.plugins))
                
                for i, plugin in enumerate(self.plugins):
                    progress.update(plugin_task, description=f"Building {plugin}...")
                    
                    success, stdout, stderr = self.run_command(
                        ["cmake", "--build", ".", "--target", plugin, "--config", self.build_config],
                        f"Building {plugin}",
                        cwd=self.build_dir
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
        
        # Verify plugin builds
        return self.verify_plugin_builds()
    
    def verify_plugin_builds(self) -> bool:
        """Verify that all plugins were built successfully."""
        self.console.print("[CHECK] Verifying plugin builds...", style="bold blue")
        
        plugin_dir = self.build_dir / "Release" / "plugins"
        
        if not plugin_dir.exists():
            self.console.print(f"[FAIL] Plugin directory not found: {plugin_dir}", style="red")
            self.logger.error(f"Plugin directory missing: {plugin_dir}")
            return False
        
        # Check each plugin DLL
        verification_table = Table(show_header=True, header_style="bold magenta")
        verification_table.add_column("Plugin", style="cyan")
        verification_table.add_column("Status", justify="center")
        verification_table.add_column("File Size", justify="right")
        verification_table.add_column("Path", style="dim")
        
        all_plugins_ok = True
        
        for plugin in self.plugins:
            dll_path = plugin_dir / f"{plugin}.dll"
            if dll_path.exists():
                file_size = dll_path.stat().st_size
                size_str = f"{file_size:,} bytes"
                verification_table.add_row(plugin, "[OK] Found", size_str, str(dll_path))
                self.logger.info(f"Plugin verified: {plugin} ({size_str})")
            else:
                verification_table.add_row(plugin, "[FAIL] Missing", "N/A", str(dll_path))
                self.logger.error(f"Plugin missing: {plugin} at {dll_path}")
                all_plugins_ok = False
        
        self.console.print(verification_table)
        self.console.print()
        
        if all_plugins_ok:
            self.console.print("[OK] All plugins verified successfully!", style="bold green")
            self.logger.info("All plugins built and verified successfully")
        else:
            self.console.print("[FAIL] Plugin verification failed!", style="bold red")
            self.logger.error("Plugin verification failed")
        
        return all_plugins_ok
    
    def step2_build_main_app(self) -> bool:
        """Step 2: Build the main application."""
        self.console.print(Panel("[MAIN] STEP 2: Building Main Application", border_style="blue", title_align="left"))
        
        # Build without using Progress to avoid Rich display conflicts
        self.console.print("[BUILD] Building ItemEditor executable...", style="bold blue")
        
        success, stdout, stderr = self.run_command(
            ["cmake", "--build", ".", "--target", "ItemEditor", "--config", self.build_config],
            "Building ItemEditor executable",
            cwd=self.build_dir
        )
        
        if not success:
            self.console.print(Panel(
                f"[red]ItemEditor build failed![/red]\n\n"
                f"Error output:\n{stderr}\n\n"
                "This could be due to:\n"
                "• Missing plugin dependencies\n"
                "• Source code compilation errors\n"
                "• Qt linking issues",
                title="[FAIL] Main Application Build Error",
                border_style="red"
            ))
            return False
        
        # Verify main executable
        exe_path = self.build_dir / "bin" / self.build_config / "ItemEditor.exe"
        if exe_path.exists():
            file_size = exe_path.stat().st_size
            self.console.print(f"[OK] ItemEditor.exe built successfully ({file_size:,} bytes)", style="green")
            self.logger.info(f"Main application built: {exe_path} ({file_size:,} bytes)")
            self.stats["main_app_built"] = True
            return True
        else:
            self.console.print(f"[FAIL] ItemEditor.exe not found at: {exe_path}", style="red")
            self.logger.error(f"Main executable not found: {exe_path}")
            return False
    
    def step3_deploy_application(self) -> bool:
        """Step 3: Deploy the complete application."""
        self.console.print(Panel("[DEPLOY] STEP 3: Deploying Application", border_style="blue", title_align="left"))
        
        # Create deployment directory
        try:
            self.deploy_dir.mkdir(parents=True, exist_ok=True)
            self.console.print(f"[CREATE] Created deployment directory: {self.deploy_dir.name}/", style="green")
        except Exception as e:
            self.console.print(f"[FAIL] Failed to create deployment directory: {e}", style="red")
            self.logger.error(f"Failed to create deployment directory: {e}")
            return False
        
        deployment_steps = [
            ("Copy main executable", self.copy_main_executable),
            ("Copy plugin DLLs", self.copy_plugins),
            ("Run Qt deployment", self.run_qt_deployment),
            ("Copy additional resources", self.copy_resources),
            ("Create convenience files", self.create_convenience_files),
            ("Final verification", self.verify_deployment)
        ]
        
        with Progress(
            SpinnerColumn(),
            TextColumn("[progress.description]{task.description}"),
            BarColumn(),
            TaskProgressColumn(),
            console=self.console
        ) as progress:
            
            deploy_task = progress.add_task("Deploying...", total=len(deployment_steps))
            
            for step_name, step_func in deployment_steps:
                progress.update(deploy_task, description=f"{step_name}...")
                
                try:
                    if not step_func():
                        self.console.print(f"[FAIL] Deployment step failed: {step_name}", style="red")
                        return False
                    self.console.print(f"[OK] {step_name}", style="green")
                except Exception as e:
                    self.console.print(f"[FAIL] Exception in {step_name}: {e}", style="red")
                    self.logger.error(f"Exception in deployment step '{step_name}': {e}")
                    return False
                
                progress.advance(deploy_task)
        
        self.stats["deployment_successful"] = True
        return True
    
    def copy_main_executable(self) -> bool:
        """Copy the main executable to deployment directory."""
        src = self.build_dir / "bin" / self.build_config / "ItemEditor.exe"
        dst = self.deploy_dir / "ItemEditor.exe"
        
        try:
            shutil.copy2(src, dst)
            self.logger.info(f"Copied main executable: {src} -> {dst}")
            return True
        except Exception as e:
            self.logger.error(f"Failed to copy main executable: {e}")
            return False
    
    def copy_plugins(self) -> bool:
        """Copy plugin DLLs to deployment directory."""
        src_dir = self.build_dir / "Release" / "plugins"
        dst_dir = self.deploy_dir / "plugins"
        
        try:
            dst_dir.mkdir(exist_ok=True)
            
            for plugin in self.plugins:
                src_dll = src_dir / f"{plugin}.dll"
                dst_dll = dst_dir / f"{plugin}.dll"
                shutil.copy2(src_dll, dst_dll)
                self.logger.info(f"Copied plugin: {src_dll} -> {dst_dll}")
            
            return True
        except Exception as e:
            self.logger.error(f"Failed to copy plugins: {e}")
            return False
    
    def run_qt_deployment(self) -> bool:
        """Run windeployqt to deploy Qt dependencies."""
        self.logger.info("Skipping manual Qt deployment. CMake will handle it.")
        return True
    
    def copy_resources(self) -> bool:
        """Copy additional resources if they exist."""
        resource_dirs = ["resources", "config"]
        
        for resource_dir in resource_dirs:
            src_dir = self.project_dir / resource_dir
            if src_dir.exists():
                dst_dir = self.deploy_dir / resource_dir
                try:
                    shutil.copytree(src_dir, dst_dir, dirs_exist_ok=True)
                    self.logger.info(f"Copied resource directory: {src_dir} -> {dst_dir}")
                except Exception as e:
                    self.logger.warning(f"Failed to copy {resource_dir}: {e}")
                    self.stats["total_warnings"] += 1
        
        return True
    
    def create_convenience_files(self) -> bool:
        """Create convenience files for users."""
        try:
            # Create test_data directory
            test_data_dir = self.deploy_dir / "test_data"
            test_data_dir.mkdir(exist_ok=True)
            
            readme_content = "Place your .otb and .spr test files in this folder\n"
            (test_data_dir / "README.txt").write_text(readme_content)
            
            # Create run script
            run_script_content = """@echo off
echo Starting Item Editor Qt6...
start ItemEditor.exe
"""
            (self.deploy_dir / "run_itemeditor.bat").write_text(run_script_content)
            
            self.logger.info("Created convenience files")
            return True
        except Exception as e:
            self.logger.error(f"Failed to create convenience files: {e}")
            return False
    
    def verify_deployment(self) -> bool:
        """Verify the deployment is complete and correct."""
        required_files = [
            ("ItemEditor.exe", "Main executable"),
            ("plugins/PluginOne.dll", "Plugin One"),
            ("plugins/PluginTwo.dll", "Plugin Two"),
            ("plugins/PluginThree.dll", "Plugin Three")
        ]
        
        verification_table = Table(show_header=True, header_style="bold magenta")
        verification_table.add_column("Component", style="cyan")
        verification_table.add_column("Status", justify="center")
        verification_table.add_column("Size", justify="right")
        
        all_files_ok = True
        
        for file_path, description in required_files:
            full_path = self.deploy_dir / file_path
            if full_path.exists():
                file_size = full_path.stat().st_size
                size_str = f"{file_size:,} bytes"
                verification_table.add_row(description, "[OK] Found", size_str)
            else:
                verification_table.add_row(description, "[FAIL] Missing", "N/A")
                all_files_ok = False
        
        self.console.print(verification_table)
        
        if all_files_ok:
            self.logger.info("Deployment verification successful")
        else:
            self.logger.error("Deployment verification failed - missing files")
        
        return all_files_ok
    
    def show_build_summary(self):
        """Display a comprehensive build summary."""
        end_time = datetime.now()
        duration = end_time - self.start_time
        
        # Create summary panel
        summary_text = Text()
        
        if self.stats["deployment_successful"]:
            summary_text.append("[SUCCESS] BUILD COMPLETED SUCCESSFULLY!\n\n", style="bold green")
        else:
            summary_text.append("[FAIL] BUILD FAILED\n\n", style="bold red")
        
        # Build statistics
        stats_table = Table(show_header=False, box=None, padding=(0, 2))
        stats_table.add_column("Metric", style="cyan", no_wrap=True)
        stats_table.add_column("Value", style="white")
        
        stats_table.add_row("[TIME] Duration:", f"{duration.total_seconds():.1f} seconds")
        stats_table.add_row("[PLUGINS] Plugins Built:", f"{self.stats['plugins_built']}/{len(self.plugins)}")
        stats_table.add_row("[MAIN] Main App:", "[OK] Success" if self.stats["main_app_built"] else "[FAIL] Failed")
        stats_table.add_row("[DEPLOY] Deployment:", "[OK] Success" if self.stats["deployment_successful"] else "[FAIL] Failed")
        stats_table.add_row("[WARN] Warnings:", str(self.stats["total_warnings"]))
        stats_table.add_row("[ERROR] Errors:", str(self.stats["total_errors"]))
        
        self.console.print(Panel(stats_table, title="[SUMMARY] Build Summary", border_style="green" if self.stats["deployment_successful"] else "red"))
        
        if self.stats["deployment_successful"]:
            # Show deployment information
            self.console.print()
            
            deploy_info = Text()
            deploy_info.append("[LOCATION] Deployment Location:\n", style="bold blue")
            deploy_info.append(f"   {self.deploy_dir}\n\n", style="white")
            deploy_info.append("[RUN] To run the application:\n", style="bold blue")
            deploy_info.append(f"   1. Navigate to: {self.deploy_dir}\n", style="white")
            deploy_info.append("   2. Double-click: ItemEditor.exe\n", style="white")
            deploy_info.append("      OR\n", style="dim")
            deploy_info.append("      Double-click: run_itemeditor.bat\n\n", style="white")
            deploy_info.append("[FILES] Test Files:\n", style="bold blue")
            deploy_info.append("   Place your .otb and .spr files in the test_data/ folder\n", style="white")
            
            self.console.print(Panel(deploy_info, title="[NEXT] Next Steps", border_style="blue"))
            
            # Show deployment contents
            self.show_deployment_contents()
        
        # Log final summary
        self.logger.info(f"Build completed in {duration.total_seconds():.1f} seconds")
        self.logger.info(f"Final status: {'SUCCESS' if self.stats['deployment_successful'] else 'FAILED'}")
        self.logger.info(f"Log file saved to: {self.log_file}")
    
    def show_deployment_contents(self):
        """Show the contents of the deployment directory in a tree view."""
        try:
            tree = Tree(f"[DEPLOY] {self.deploy_dir.name}/", style="bold blue")
            
            # Add files and directories
            for item in sorted(self.deploy_dir.iterdir()):
                if item.is_file():
                    size = item.stat().st_size
                    tree.add(f"[FILE] {item.name} ({size:,} bytes)")
                elif item.is_dir():
                    dir_node = tree.add(f"[DIR] {item.name}/")
                    try:
                        for subitem in sorted(item.iterdir()):
                            if subitem.is_file():
                                size = subitem.stat().st_size
                                dir_node.add(f"[FILE] {subitem.name} ({size:,} bytes)")
                    except PermissionError:
                        dir_node.add("[FAIL] Permission denied")
            
            self.console.print(Panel(tree, title="[CONTENTS] Deployment Contents", border_style="cyan"))
            
        except Exception as e:
            self.console.print(f"[WARN] Could not display deployment contents: {e}", style="yellow")
    
    def run_build(self) -> bool:
        """Run the complete build process."""
        try:
            self.show_header()
            
            # Step 0: Prerequisites
            if not self.verify_prerequisites():
                return False
            
            # Step 0.5: Clean
            if not self.clean_previous_builds():
                return False
            
            # Step 1: Build plugins
            if not self.step1_build_plugins():
                return False
            
            # Step 2: Build main application
            if not self.step2_build_main_app():
                return False
            
            # Step 3: Deploy application
            if not self.step3_deploy_application():
                return False
            
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
    """Main entry point."""
    console = Console()
    
    # Check Python version
    if sys.version_info < (3, 7):
        console.print("[FAIL] Python 3.7 or higher is required", style="bold red")
        sys.exit(1)
    
    # Create and run builder
    builder = Qt6Builder()
    success = builder.run_build()
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()