# Project Guidelines

## 1. Code Simplicity
- Keep code as simple as possible
- Performance overhead should be less than or equal to linear
- Prioritize readability and maintainability over minor performance optimizations

## 2. Access and Memory Management
- Use public access for all class members by default
- Avoid const modifiers
- Prefer pass-by-value and copying over references
- Only use references when absolutely necessary
- When references or move operations are needed, notify for proper implementation of copy constructors and move constructors

## 3. C++ Features
- Advanced C++ features are acceptable when they contribute to code simplicity
- Use modern C++ features if they make the code more readable or maintainable
- Don't avoid newer language features just because they're advanced - if they simplify the code, use them

## Summary of Best Practices
1. Simple > Complex
2. Linear performance is acceptable
3. Public by default
4. Pass by value > Pass by reference
5. Modern C++ features are welcome if they simplify code
6. Always prioritize readability and maintainability
