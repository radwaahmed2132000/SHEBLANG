- Clean up code
  - Rename Nodes to perhasp remove the `Node` suffix.
  - Clean up semantic analyser code (perhaps use libfmt for easier printing?).
  - Clean up compiler code.
  - Clean up scope setup code.

- Array support?
- Namespaces / modules
  ```
  module foo {
    fn bar(int baz) int { ... }
  }

  module qux {
    fn bar(string corge) float { ... }
  }

  fn main() {
    foo::bar(12);
    qux::bar("grault");
  }
  ```

- Check if a symbol is defined as multiple symbol types (var + fn, enum + fn, etc ...)
- Re-enable break statements inside switch cases to allow for early exits. This wouldn't change the fact that switch cases automatically break on case block completion.
- Run testcases in parallel.