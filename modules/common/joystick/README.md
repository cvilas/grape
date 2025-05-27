# README: joystick

## Brief

joystick provides .. <one line description>

## Detailed description

See documentation inline

## TODO

- Avoid exceptions everywhere. use `std::expected` and `std::optional`
- Replace exception_ptr with std::error_code
- Consider moving ScopeGuard to utils
- Should we open joystick read-only or read-write (to set axis range)
- Replace all uses if `strerror` with `const auto err = std::error_code(errno, std::system_category()); err.message();` 
- Fix all TODOs