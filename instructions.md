# Migration Instructions: Legacy ItemEditor to Modern .NET 9 Application

You are an AI coding assistant that has to plan tasks to migrate the "legacy-app" to a new, better state-of-the-art .NET 9 application in the "net9-project" folder, providing 100% project completion and 1:1 function parity. The migration must maintain identical functionality while modernizing the technology stack and implementing a beautiful, contemporary user interface.

## Migration Objectives

### Primary Goals
- **Complete Function Parity**: Every feature, tool, and capability from the legacy application must be preserved
- **Modern Technology Stack**: Upgrade from .NET Framework 4.8 to .NET 9 with latest C# features
- **Beautiful UI Design**: Replace Windows Forms + native theming with modern WPF + WPFUI design system
- **Identical File Structure**: Maintain same project organization, file names, and folder hierarchy
- **Enhanced Performance**: Leverage .NET 9 performance improvements and modern patterns
- **Future-Ready Architecture**: Design for extensibility and modern development practices

### Core Requirements
- **File Format Support**: Maintain 100% compatibility with .dat, .spr, .otb file handling and parsing
- **Graphics Rendering**: Preserve all image display, sprite rendering, and visual capabilities
- **Plugin System**: Modernize but maintain complete plugin architecture compatibility
- **Data Processing**: Keep all item editing, comparison, and manipulation features
- **User Experience**: Improve UI/UX while maintaining familiar workflow patterns

## Technology Migration Map

### Framework Transition
- **From**: .NET Framework 4.8 + Windows Forms + DarkUI
- **To**: .NET 9 + WPF + WPFUI + Modern C# patterns + Custom Theme Management

### UI Framework Migration
- **Legacy**: Windows Forms controls with DarkUI theming (being replaced with custom theme)
- **Modern**: WPF with WPFUI design system + custom theme management
- **Design Principles**: Fluent Design, modern spacing, contemporary typography, responsive layouts, custom light/dark theme support
- **Design Standards**: Premium user experience with comprehensive design system integration

### Dependency Modernization
- **System.Drawing.Common** → **WPF Imaging** + **SkiaSharp** (for advanced graphics)
- **Windows Forms Controls** → **WPFUI Controls** + **Custom WPF Controls**
- **DarkUI Library** → **Custom Light/Dark Theme System** + **WPFUI Theme Integration**
- **Legacy File I/O** → **Modern async/await patterns** with **System.IO.Pipelines**
- **Plugin System** → **Modern dependency injection** with **Microsoft.Extensions.DependencyInjection**

## Project Structure Requirements

### Folder Hierarchy (1:1 Mapping)
```
net9-project/
├── Source/                          # Main application (mirrors legacy structure)
│   ├── ItemEditor.csproj            # Modern .NET 9 project file
│   ├── Program.cs                   # Application entry point
│   ├── MainWindow.xaml/.cs          # Main window (WPF equivalent of MainForm)
│   ├── App.xaml/.cs                 # Application configuration
│   ├── Controls/                    # Custom WPF controls
│   │   ├── ClientItemView.xaml/.cs  # WPF version of ClientItemView
│   │   ├── ServerItemListBox.xaml/.cs
│   │   └── FlagCheckBox.xaml/.cs
│   ├── Dialogs/                     # WPF dialogs and windows
│   │   ├── AboutWindow.xaml/.cs     # Modern dialog implementations
│   │   ├── CompareOtbWindow.xaml/.cs
│   │   ├── FindItemWindow.xaml/.cs
│   │   ├── NewOtbFileWindow.xaml/.cs
│   │   ├── PreferencesWindow.xaml/.cs
│   │   ├── ProgressWindow.xaml/.cs
│   │   ├── UpdateWindow.xaml/.cs
│   │   └── UpdateSettingsWindow.xaml/.cs
│   ├── Helpers/                     # Utility classes (modernized)
│   │   ├── FileNameHelper.cs
│   │   ├── PathHelper.cs
│   │   └── Utils.cs
│   ├── Host/                        # Plugin hosting system
│   │   ├── Plugin.cs
│   │   ├── PluginCollection.cs
│   │   └── PluginServices.cs
│   ├── Services/                    # Modern service layer
│   │   ├── IFileService.cs
│   │   ├── IImageService.cs
│   │   └── IPluginService.cs
│   ├── ViewModels/                  # MVVM pattern implementation
│   │   ├── MainViewModel.cs
│   │   ├── ItemViewModel.cs
│   │   └── DialogViewModels/
│   ├── Themes/                      # WPFUI themes and styles
│   │   ├── Generic.xaml
│   │   └── ItemEditorTheme.xaml
│   └── Resources/                   # Application resources
│       ├── Images/                  # Converted image resources
│       ├── Styles/                  # WPFUI style definitions
│       └── Converters/              # WPF value converters
├── PluginInterface/                 # Plugin system (modernized)
│   ├── PluginInterface.csproj
│   ├── IPlugin.cs                   # Updated plugin contracts
│   ├── Item.cs                      # Core item classes
│   ├── Settings.cs
│   ├── Sprite.cs
│   ├── ImageSimilarity/             # Image processing utilities
│   └── OTLib/                       # OpenTibia library
│       ├── Collections/
│       ├── OTB/                     # OTB file handling
│       ├── Server/Items/            # Server item management
│       └── Utils/                   # Utility classes
├── PluginOne/                       # Example plugins (modernized)
├── PluginTwo/
├── PluginThree/
└── ThirdParty/                      # Modern dependencies
    └── WPFUI/                       # WPFUI integration with custom theme support
```

## Migration Planning Framework

### Phase 1: Foundation Setup
1. **Project Structure Creation**
   - Create .NET 9 WPF project with WPFUI
   - Set up modern project file with package references
   - Establish folder structure matching legacy layout
   - Configure build system and dependencies

2. **Core Infrastructure**
   - Implement modern dependency injection container
   - Set up logging framework (Microsoft.Extensions.Logging)
   - Create service abstractions for file operations
   - Establish MVVM pattern foundation

### Phase 2: Core Library Migration
1. **PluginInterface Modernization**
   - Update to .NET 9 with modern C# features
   - Maintain API compatibility while adding async support
   - Modernize OTLib components with performance improvements
   - Update ImageSimilarity with latest algorithms

2. **File Format Handlers**
   - Migrate OTB reader/writer with async/await patterns
   - Update .dat/.spr file parsing with modern I/O
   - Implement streaming for large file operations
   - Add progress reporting for long operations

### Phase 3: UI Framework Migration
1. **WPFUI Integration**
   - Set up WPFUI theme and styling system
   - Create base window and control templates
   - Implement modern navigation patterns
   - Design responsive layout system

2. **Control Migration**
   - Convert Windows Forms controls to WPF+WPFUI equivalents
   - Maintain exact functionality while improving aesthetics
   - Implement proper data binding patterns
   - Add modern interaction patterns (drag-drop, context menus)

### Phase 4: Application Logic Migration
1. **Main Application**
   - Convert MainForm to MainWindow with MVVM
   - Implement modern menu and toolbar systems
   - Add status bar with modern progress indicators
   - Maintain all existing functionality

2. **Dialog Systems**
   - Convert all dialogs to modern WPF windows
   - Implement proper modal/modeless behavior
   - Add modern validation and error handling
   - Maintain exact user workflows

### Phase 5: Plugin System Modernization
1. **Plugin Architecture**
   - Update plugin loading with modern assembly loading
   - Implement plugin isolation and security
   - Add plugin dependency management
   - Maintain backward compatibility

2. **Plugin Examples**
   - Modernize example plugins as templates
   - Document new plugin development patterns
   - Provide migration guide for existing plugins

## Technical Implementation Guidelines

### Modern C# Patterns
- **Async/Await**: Use throughout for file operations and long-running tasks
- **Nullable Reference Types**: Enable and properly annotate all code
- **Pattern Matching**: Leverage modern C# pattern matching features
- **Records**: Use for immutable data structures where appropriate
- **Source Generators**: Consider for performance-critical code paths

### WPF + WPFUI Best Practices
- **MVVM Pattern**: Strict separation of concerns with ViewModels
- **Data Binding**: Leverage WPF's powerful binding system
- **Commands**: Use ICommand pattern for all user interactions
- **Styles and Templates**: Create reusable, themeable components
- **Resource Management**: Proper disposal of graphics resources

### Performance Considerations
- **Memory Management**: Implement proper disposal patterns
- **Large File Handling**: Use streaming and chunked processing
- **UI Responsiveness**: Keep UI thread free with proper async patterns
- **Caching**: Implement intelligent caching for frequently accessed data

### Error Handling Strategy
- **Structured Logging**: Use Microsoft.Extensions.Logging throughout
- **Exception Handling**: Implement global exception handling
- **User Feedback**: Provide clear, actionable error messages
- **Recovery Mechanisms**: Allow graceful recovery from errors

## UI/UX Design Requirements - Premium Professional Standards

### Foundation: Visual Hierarchy & Typography Excellence

#### Typography System Implementation
- **Typographic Scale**: Establish proper scale using 1.25 ratio (12px, 15px, 18px, 24px, 30px, 37px, 46px)
- **Font Weights**: Consistent weights with clear purpose (300/light for subtle text, 400/regular for body, 600/semibold for emphasis, 700/bold for headings)
- **Line Heights**: Perfect readability (1.2 for headings, 1.5 for body text, 1.4 for UI elements)
- **Letter Spacing**: Optimal spacing (-0.02em for large text, 0 for body, +0.05em for small caps)
- **Information Hierarchy**: Create clear relationships using size, color, and spacing
- **ItemEditor Application**: Use larger scales for item names, smaller for properties, consistent sizing for plugin interfaces

#### Color & Contrast Optimization
- **WCAG AA Compliance**: Audit all color combinations for 4.5:1 contrast minimum for accessibility
- **Cohesive Color Palette**: Establish primary, secondary, and semantic colors with consistent variants (50, 100, 200, 300, 400, 500, 600, 700, 800, 900)
- **Perfect Color Usage**: Primary for actions/CTAs, secondary for information, semantic for status (success, warning, error)
- **Dark Mode Compatibility**: Ensure appropriate color inversions and contrast maintenance
- **ItemEditor Color System**: 
  - Plugin status indicators (active = green, inactive = gray, error = red)

#### Spacing & Layout Refinement
- **Consistent Spacing System**: Implement 4px/8px base unit system (4, 8, 12, 16, 24, 32, 48, 64px) for all margins and padding
- **Perfect Component Padding**: Balance visual hierarchy with breathing room
- **Border Radius Values**: Establish consistent values (2px for inputs, 4px for buttons, 8px for cards, 12px for modals, 16px for major containers)
- **White Space Distribution**: Optimize for focus and visual balance
- **Grid System Alignment**: Align all elements to invisible grid for precision
- **ItemEditor Layout Optimization**:
  - Item list optimal density vs readability
  - Property editor logical grouping with proper spacing
  - Tool palette efficient organization

#### Component Standardization Excellence
- **Button Unification**: Consistent heights (32px for compact, 40px for standard, 48px for prominent) with unified padding and interaction states
- **Form Input Standards**: Standardize sizing, spacing, and all interaction states (default, hover, focus, disabled, error)
- **Icon System Perfection**: Consistent sizing (16px for inline, 20px for buttons, 24px for navigation, 32px for features) with optical balance
- **Card/Container Styling**: Unified shadows, borders, and elevation system for visual hierarchy
- **Loading & State Designs**: Cohesive loading, empty, and error state presentations
- **ItemEditor Component Library**:
  - Standardized item preview cards with consistent dimensions
  - Unified property editor controls (text inputs, dropdowns, checkboxes)
  - Consistent dialog and modal styling
  - Standardized toolbar and menu components
  - Unified progress indicators for file operations

### Premium User Experience: Micro-Interactions & Animations

#### Sophisticated Interaction Design
- **Subtle Hover States**: Add 0.2s ease transitions to all interactive elements for premium feel
- **Focus State Excellence**: Implement visible accessibility-compliant focus indicators with subtle glow effects
- **Loading Animation System**: Create smooth loading animations using skeleton screens or branded spinners
- **Page Transition Elegance**: Add gentle transitions (slide, fade) between major sections
- **Micro-Feedback Implementation**: Include delightful micro-feedback for user actions (button press effects, form submissions)
- **Contextual Tooltips**: Create helpful tooltips that appear on hover/focus with 0.3s delay

#### Navigation & User Flow Optimization
- **Perfect Breadcrumb Navigation**: Show clear user location in file/plugin hierarchy with clickable path segments
- **Intelligent Search Implementation**: Smart search with autocomplete, recent searches, and filtering suggestions
- **Contextual Help System**: Add onboarding hints and contextual help for complex OTB editing features
- **Intuitive Navigation Patterns**: Clear back/forward navigation with logical flow
- **Call-to-Action Hierarchy**: Design clear primary/secondary action distinction
- **Smart Form Flows**: Optimize field progression with intelligent validation and auto-completion
- **Progress Indication**: Include detailed progress indicators for multi-step file operations
- **ItemEditor Navigation Excellence**:
  - File browser with breadcrumb navigation
  - Property navigation with logical grouping
  - Batch operation progress with detailed status

### Performance Optimization Excellence

#### Advanced Performance Features
- **Lazy Loading Implementation**: Implement lazy loading for images and heavy content sections
- **Skeleton Screen Mastery**: Add skeleton screens for anticipated loading states
- **Image Optimization**: Perfect image optimization with appropriate sizes and formats
- **Infinite Scroll Patterns**: Create efficient infinite scroll or pagination for large item lists
- **Critical Path Optimization**: Optimize critical rendering path elements
- **Preloading Intelligence**: Include preloading for likely user actions
- **Progressive Enhancement**: Design progressive enhancement for slower connections
- **ItemEditor Performance**:
  - Lazy load item sprites and thumbnails
  - Efficient virtual scrolling for large item databases
  - Background processing for file operations
  - Memory-efficient plugin management

### Advanced Visual Effects & Polish

#### Sophisticated Visual Design
- **Shadow System Excellence**: Add sophisticated shadow systems with multiple elevation levels
- **Glassmorphism Effects**: Implement glassmorphism effects on modal overlays and navigation
- **Gradient Sophistication**: Create subtle gradient overlays using brand colors (10-15% opacity)
- **Texture & Depth**: Add texture and depth with subtle noise patterns or grain effects
- **Backdrop Blur Mastery**: Perfect backdrop blur effects for layered interfaces
- **Parallax Scrolling**: Include subtle parallax scrolling for hero sections where appropriate
- **Premium Card Styling**: Design premium card styling with multiple shadow layers
- **Elegant Separators**: Add elegant dividers and separators with gradient fades
- **ItemEditor Visual Polish**:
  - Elevated item preview cards with sophisticated shadows
  - Glassmorphism modal overlays for dialogs
  - Subtle texture on main interface panels
  - Premium loading animations with brand personality

#### Enterprise-Grade Polish & Features
- **Help Documentation Integration**: Add comprehensive help documentation integration
- **Advanced Search & Filter**: Include advanced search and filtering capabilities
- **Import/Export Workflows**: Perfect data import/export workflows with validation

## Quality Assurance Requirements

### Testing Strategy
- **Unit Tests**: Cover all business logic with comprehensive tests
- **Integration Tests**: Test file format compatibility
- **UI Tests**: Automated testing of user workflows
- **Performance Tests**: Ensure no regression in performance
- **Plugin Tests**: Verify plugin system compatibility

### Compatibility Verification
- **File Format Tests**: Verify 100% compatibility with existing files
- **Feature Parity Tests**: Ensure all legacy features work identically
- **Plugin Compatibility**: Test existing plugins work with new system
- **Performance Benchmarks**: Compare performance with legacy version

### Documentation Requirements
- **Migration Guide**: Document changes for users
- **Developer Documentation**: API changes and new patterns
- **Plugin Development Guide**: Updated plugin development instructions
- **Deployment Guide**: Modern deployment and installation procedures

## Success Criteria

### Functional Requirements
- [ ] 100% feature parity with legacy application
- [ ] All file formats (.dat, .spr, .otb) work identically
- [ ] All existing plugins function without modification
- [ ] Performance equal to or better than legacy version
- [ ] Modern, beautiful UI that improves user experience

### Technical Requirements
- [ ] .NET 9 with modern C# features
- [ ] WPF + WPFUI for contemporary UI design
- [ ] Proper async/await patterns throughout
- [ ] Comprehensive test coverage
- [ ] Modern deployment and installation

### Quality Requirements
- [ ] Zero regression in functionality
- [ ] Improved startup and operation performance
- [ ] Enhanced error handling and user feedback
- [ ] Maintainable, extensible codebase
- [ ] Future-ready architecture for new features

## Implementation Timeline

### Week 1-2: Foundation
- Project setup and infrastructure
- Core library structure
- Basic WPFUI integration

### Week 3-4: Core Migration
- File format handlers
- Plugin interface modernization
- Basic UI framework

### Week 5-6: UI Implementation
- Main window and dialogs
- Control migration
- Theme implementation

### Week 7-8: Integration & Testing
- Plugin system integration
- Comprehensive testing
- Performance optimization

### Week 9-10: Polish & Documentation
- UI refinement
- Documentation completion
- Final testing and validation

## Deliverables

1. **Complete .NET 9 Application** in `net9-project` folder
2. **Migration Documentation** detailing all changes
3. **Test Suite** ensuring 100% compatibility
4. **Plugin Development Guide** for modern plugin creation
5. **Deployment Package** ready for distribution

This migration will result in a modern, maintainable, and beautiful application that preserves all existing functionality while providing a foundation for future enhancements and features.