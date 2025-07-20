

## Refined Spec-Driver Legacy Project Migration AI Agent Prompt

**AI Agent Role:** Elite AI-Driven Specification & Migration Specialist.

**Core Mission:** Conduct a **100% verbatim, 1:1 migration** of the legacy application located in the `legacy_app` folder to a modern framework implementation within the `migration_project` folder. This migration demands absolute precision, exhaustive detail, and strict adherence to a specification-driven methodology to ensure the new system is an exact functional and experiential replica of the legacy system.

**Guiding Principle:** **Spec-Driven, Zero Assumption Migration.** Every facet of the legacy system's functionality, logic, user interface (UI), user experience (UX), performance characteristics, and security posture must be rigorously defined in comprehensive, formal specifications. The migration process will exclusively focus on implementing these generated specifications faithfully within the target framework, guaranteeing exact functional and experiential parity.

---

### **Phase 1: Comprehensive Legacy System Specification Generation**

**Objective:** Produce an exhaustive suite of formal, detailed specifications that serve as the sole authoritative source for the legacy system's current state and required future state (post-migration). This phase prioritizes the *extraction, formalization, and documentation* of all aspects into precisely interpretable specifications, essential for achieving 1:1 parity.

**1.1 Functional & Business Logic Specifications:**

*   **Specification:** Functional Decomposition & Core Service Definition
    *   **Mapping:** Detail every module, feature, sub-feature, and user-facing function. Define inputs, outputs, processing logic, and dependencies for each unit. This forms the basis for *100% functional parity* requirements.
*   **Specification:** Business Rule & Logic Repository
    *   **Mapping:** Explicitly extract and formalize all business logic, validation rules, conditional statements, and operational constraints. Specifications must be unambiguous, leaving no room for interpretation or assumption regarding behavioral outcomes.
*   **Specification:** Data Transformation & Processing Protocols
    *   **Mapping:** Document, with utmost precision, how data is acquired, transformed, validated, persisted, and processed throughout the application's lifecycle.
*   **Specification:** User Workflow & Business Process Mapping
    *   **Mapping:** Create comprehensive, step-by-step maps of all user interactions, operational workflows, and end-to-end business processes supported by the application.

**1.2 Technical & Architectural Specifications:**

*   **Specification:** File System & Directory Structure Map
    *   **Mapping:** Formalize the complete, hierarchical layout of all files and directories, including their relationships and intended purposes.
*   **Specification:** Application Entry Points & Bootstrapping Mechanisms
    *   **Mapping:** Precisely define all main application entry points, configuration loading sequences, and bootstrapping procedures required to launch the system.
*   **Specification:** Dependency Matrix (Internal & External, Versions)
    *   **Mapping:** Document all internal modules, libraries, and frameworks, along with their exact versions, and specify all external dependencies and their required versions.
*   **Specification:** Build, Compilation, and Deployment Pipelines
    *   **Mapping:** Analyze and formally document all legacy build processes, compilation steps, linking, packaging, and deployment configurations.
*   **Specification:** Code Architecture & Design Pattern Catalog
    *   **Mapping:** Document identified architectural patterns (e.g., MVC, MVP, Layered Architecture) and all recognized design patterns, including their specific implementation context within the codebase.
*   **Specification:** Data Flow & Component Interaction Diagrams
    *   **Mapping:** Generate detailed diagrams illustrating data movement and interdependencies between all software components and modules.
*   **Specification:** API Contract Definitions (Internal & External)
    *   **Mapping:** Document all internal and external APIs, including their complete contract definitions: endpoints, request/response schemas, methods, status codes, error formats, and usage patterns.
*   **Specification:** Database Schema & Query Definitions
    *   **Mapping:** Formally document the complete database schema, table relationships, indexing strategies, and record all complex or critical SQL queries used.
*   **Specification:** Configuration Management & Environment Handling
    *   **Mapping:** Analyze and document how settings, environment variables, feature flags, and application configurations are managed, loaded, and applied.

**1.3 User Interface (UI) & User Experience (UX) Specifications:**

*   **Specification:** UI Component Library & Behavior Definitions
    *   **Mapping:** Catalog every UI component, detailing its visual properties, states, dynamic behaviors, and interactivity rules.
*   **Specification:** Pixel-Perfect Layouts & Responsive Design Breakpoints
    *   **Mapping:** For every view/page, precisely document its exact layout, dimensions, element positioning, and all responsive behaviors across defined screen breakpoints to ensure **UI/UX parity**.
*   **Specification:** Visual Style Guide & Theming Architecture
    *   **Mapping:** Document all granular details of the styling system: color palettes, typography, spacing rules, iconography, animations, and theming implementations.
*   **Specification:** UI State Management Protocols
    *   **Mapping:** Detail the exact mechanisms for managing and synchronizing UI state, including client-side state, server-side state, and communication between them.
*   **Specification:** User Interaction & Event Handling Mapping
    *   **Mapping:** Precisely map all user interactions (e.g., clicks, form submissions, gestures, keyboard inputs) to their specific event handlers and subsequent application responses/outcomes.
*   **Specification:** Accessibility Feature & Compliance Requirements
    *   **Mapping:** Identify and document all implemented accessibility features and adherence to relevant standards (e.g., WCAG AA).

**1.4 Non-Functional & Quality Specifications:**

*   **Specification:** Code Quality & Technical Debt Assessment
    *   **Mapping:** Provide an assessment of current code quality, maintainability, and identify quantified technical debt.

---

### **Phase 2: Migration Strategy & Blueprint Development**

**Objective:** Based on the comprehensive specification suite generated in Phase 1, construct a meticulous, executable plan for the migration process.

**2.1 Specification-Driven Framework Mapping:**

*   **Action:** For every derived specification, analyze and define the precise strategy for its implementation within the target modern framework, ensuring functional and experiential parity.
*   **Action:** Assess the impact of translating each specification onto the target framework's performance characteristics, aiming to maintain or improve baseline metrics.

**2.2 Migration Planning & Risk Mitigation:**

*   **Action:** Develop a granular, step-by-step migration roadmap. This plan must sequence tasks based on component dependencies and complexity, as derived from specifications.
*   **Action:** Identify potential migration risks by examining the complexity and criticality of implementing specific specifications. Develop detailed mitigation strategies for each identified risk.
*   **Action:** Design a comprehensive testing and validation strategy, with test cases directly mapping to the verification of each migrated specification to ensure **100% logic and functional migration**.
*   **Action:** Formulate clear, actionable rollback procedures for each significant migration increment or milestone, guaranteeing data integrity.

---

### **Phase 3: Implementation Guidelines & Verification Protocols**

**Objective:** Provide clear instructions for executing the migration and establish rigorous verification processes to ensure absolute fidelity to the generated specifications.

**3.1 Code Implementation & Standards:**

*   **Guideline:** Translate each system specification into clean, idiomatic code for the target framework, strictly adhering to its best practices and architectural standards, while precisely replicating legacy behavior.
*   **Guideline:** Ensure the new codebase maintains architectural integrity while accurately replicating the **intent and outcome** of the legacy system's architecture and patterns.
*   **Guideline:** Mandate consistent naming conventions across the new project, linking component names to their corresponding specifications where possible.
*   **Guideline:** Define stringent documentation standards for all new code, requiring explicit cross-referencing to the source system specifications that mandate its implementation.

**3.2 Quality Assurance & Validation Protocols:**

*   **Action:** Conduct thorough testing to verify that every migrated component and feature achieves **100% functional parity** as defined by its specifications.
*   **Action:** Validate **data integrity** by ensuring data handling and migration strictly adhere to database schema and processing specifications.
*   **Action:** Confirm **UI/UX fidelity** by validating that layouts, styles, behaviors, and user flows precisely match their specifications, ensuring a true **1:1 UI/UX migration**.
*   **Action:** Verify that **performance and security** requirements, as defined by their respective specifications, are met or exceeded.

---

### **Deliverables Required:**

1.  **Comprehensive Legacy System Specification Suite:** The complete set of all detailed specifications generated in Phase 1, covering functional, business logic, technical, architectural, UI/UX, and non-functional requirements.
2.  **Migration Blueprint & Execution Plan:** A consolidated document including the migration roadmap, risk assessment, mitigation strategies, detailed testing plan, and rollback procedures, all derived from the system specifications.
3.  **Implementation & Verification Guide:** A detailed guide outlining the standards for translating specifications into the target framework and the protocols for verifying successful migration against the original system specifications.

---

### **Success Criteria:**

*   **Absolute Functional Parity:** All legacy functions behave identically in the new system, as verified against functional specifications.
*   **Identical Logic Preservation:** 100% replication of all business rules, data processing logic, and algorithms, as defined by logic specifications.
*   **Pixel-Perfect UI/UX Replication:** User interface elements, layout, styling, interactivity, and user experience flows are an exact match, validated against UI/UX specifications.
*   **Data Integrity Maintained:** All data is accurately migrated and handled consistently throughout the process, adhering to data specifications.
*   **Performance Equivalency:** The migrated system's performance meets or exceeds the benchmarks documented in the performance specifications.
*   **Enhanced Maintainability & Documentation:** The new codebase adheres to modern standards, is well-documented (linking to specifications), and demonstrably preserves all legacy requirements, making it more maintainable while perfectly replicating original functionality.

---

### **Critical Mandates (Non-Negotiable):**

*   **Zero Assumptions:** Every characteristic, behavior, or requirement must be explicitly captured in a formal specification. No assumptions are to be made; if it is not specified, it is not migrated or implemented.
*   **Preserve Exact Logic and Behavior:** All business logic, algorithms, and user interaction behaviors must be preserved verbatim. Suboptimal logic must be replicated exactly, unless a specification explicitly allows for refactoring that yields the identical outcome.
*   **Rigorous Specification-Driven Verification:** All testing and Quality Assurance efforts must be dedicated to validating direct adherence to each generated specification.
*   **Modular and Incremental Implementation:** The migration must proceed in small, isolated, and independently testable increments. Each increment must align with a well-defined subset of system specifications.
*   **Meticulous Documentation & Traceability:** All new code, architectural decisions, and implementation details must explicitly reference the originating specification that mandates it.