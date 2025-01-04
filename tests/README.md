# foowrite tests

All tests behave in a similar way:

- Write some text
- Run some operation
- Verify the operation has changed the text in the right way.

Most tests behave exactly as how vim would, the only exception (unless I forget to update this README)
is key up on the first line: it goes to the beginning of the line because it made long lines behave
better.

To achieve that there is a small helper in `editor_stubs.cc` that can handle simple strings.
Not all operations are tested yet.