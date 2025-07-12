# Plugin Testing Framework Documentation

This directory contains a comprehensive testing framework for the Qt6 plugin system. The framework provides extensive test coverage for all aspects of the plugin architecture.

## Test Structure

### Core Test Files

1. **test_plugins.cpp** - Main comprehensive plugin system test suite
   - Plugin loading and unloading tests
   - Interface compliance validation
   - Plugin lifecycle management
   - Error handling and recovery
   - Security and isolation tests

2. **test_plugin_loader.cpp** - PluginLoader class unit tests
   - Plugin loading mechanisms
   - Symbol resolution and validation
   - Configuration management
   - Statistics and error reporting
   - Timeout and cancellation handling

3. **test_plugin_manager.cpp** - PluginManager class unit tests
   - Plugin discovery and management
   - Host service implementation
   - Inter-plugin communication
   - Configuration and logging services
   - Signal and event handling

4. **test_plugin_performance.cpp** - Performance and stress tests
   - Loading/unloading performance benchmarks
   - Memory usage and leak detection
   - Concurrent access testing
   - Scalability testing
   - Resource usage monitoring

5. **test_plugin_integration.cpp** - Integration tests
   - End-to-end plugin lifecycle testing
   - System component integration
   - Real-world usage scenarios
   - Error propagation testing
   - Complete system startup/shutdown

## Test Categories

### Unit Tests
- Individual component testing
- Interface compliance verification
- Error condition handling
- Configuration management

### Integration Tests
- Component interaction testing
- System-wide functionality
- Plugin communication chains
- Host service integration

### Performance Tests
- Loading/unloading benchmarks
- Memory usage monitoring
- Concurrent access patterns
- Scalability measurements

### Stress Tests
- Repeated operations
- High-load scenarios
- Resource exhaustion testing
- Recovery mechanisms

## Running Tests

### Build Tests
```bash
cd project_qt6/tests
mkdir build && cd build
cmake ..
make
```

### Run Individual Test Suites
```bash
./PluginSystemTests
./PluginLoaderTests
./PluginManagerTests
./PluginPerformanceTests
./PluginIntegrationTests
```

### Run All Plugin Tests
```bash
# Run all plugin-related tests
for test in Plugin*Tests; do
    echo "Running $test..."
    ./$test
done
```

## Test Features

### Comprehensive Coverage
- **Plugin Loading**: Dynamic loading, static plugins, batch operations
- **Interface Validation**: IPlugin, IPluginHost, ClientItems compliance
- **Lifecycle Management**: Initialize, dispose, reload scenarios
- **Error Handling**: Invalid plugins, timeouts, crashes, recovery
- **Performance**: Load times, memory usage, concurrent access
- **Security**: Sandboxing, permissions, isolation
- **Integration**: Component interaction, real-world scenarios

### Mock Plugin Support
- Creates temporary test plugins for framework testing
- Simulates various plugin types and versions
- Tests error conditions with invalid plugins
- Provides controlled test environment

### Real Plugin Testing
- Automatically detects real plugin files
- Tests actual plugin functionality when available
- Validates real client loading scenarios
- Verifies compatibility with existing plugins

### Performance Monitoring
- Benchmarks loading/unloading operations
- Monitors memory usage and leak detection
- Tests concurrent access patterns
- Measures scalability characteristics

### Thread Safety Testing
- Concurrent plugin operations
- Thread-safe access validation
- Race condition detection
- Deadlock prevention verification

## Test Configuration

### Environment Variables
- `PLUGIN_TEST_PATH`: Override default test plugin directory
- `PLUGIN_TEST_TIMEOUT`: Set custom timeout values
- `PLUGIN_TEST_VERBOSE`: Enable verbose test output

### CMake Options
- `ENABLE_PLUGIN_TESTS`: Enable/disable plugin testing (default: ON)
- `PLUGIN_TEST_COVERAGE`: Enable coverage reporting
- `PLUGIN_TEST_VALGRIND`: Enable memory checking

## Expected Behavior

### With Mock Plugins
- Framework tests pass (loading mechanisms work)
- Plugin loading fails gracefully (mock plugins are invalid)
- Error handling is verified
- Performance characteristics are measured

### With Real Plugins
- Complete functionality testing
- Actual plugin loading and usage
- Client file validation
- Real-world scenario verification

## Test Results Interpretation

### Success Criteria
- All framework mechanisms work correctly
- Error handling is robust
- Performance meets thresholds
- Memory usage is reasonable
- Thread safety is maintained

### Common Issues
- Mock plugin loading failures (expected)
- Performance variations on different systems
- Platform-specific behavior differences
- Real plugin availability dependencies

## Extending Tests

### Adding New Test Cases
1. Add test methods to appropriate test class
2. Follow naming convention: `test[Feature][Scenario]()`
3. Use QVERIFY/QCOMPARE for assertions
4. Add cleanup in test method if needed

### Adding New Test Classes
1. Create new test file following naming pattern
2. Inherit from QObject
3. Add private slots for test methods
4. Include QTEST_MAIN macro
5. Update CMakeLists.txt

### Performance Test Guidelines
- Use QElapsedTimer for timing measurements
- Set reasonable performance thresholds
- Account for system variations
- Measure multiple iterations for accuracy

## Dependencies

### Qt6 Components
- Qt6::Core - Core functionality
- Qt6::Test - Testing framework
- Qt6::Gui - GUI components (for plugin interfaces)
- Qt6::Concurrent - Concurrent operations testing

### Project Components
- Plugin system headers (include/plugins/)
- Plugin implementation (src/plugins/)
- OTB system components (for plugin data types)

## Maintenance

### Regular Tasks
- Update performance thresholds as needed
- Add tests for new plugin features
- Verify compatibility with new Qt versions
- Update mock plugins for new scenarios

### Debugging Tests
- Use Qt Creator debugger for test debugging
- Enable verbose output for detailed information
- Check test logs for specific failure reasons
- Verify test environment setup

## Notes

- Tests are designed to work with or without real plugins
- Mock plugins simulate various scenarios safely
- Performance tests may vary based on system capabilities
- Integration tests verify complete system behavior
- All tests include proper cleanup to prevent interference