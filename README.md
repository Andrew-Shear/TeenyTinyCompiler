Inspired by Austin Z. Henley's "Let's make a Teeny Tiny compiler" project. I initially implemented his design in Python, but then decided to rewrite it in C before adding more features. Why? Because I hate fun. More information can be found [here](https://austinhenley.com/blog/teenytinycompiler1.html).

The grammar for this language is very simple, as it has very few features, but it will be updated as the featureset grows. Example programs can be found in the `examples` directory, with example.teeny displaying the particulars of how types are handled.

To run:

`make all` -- compiles all .teeny files in examples or in the main folder.

Then all .teeny files will be executables. Enjoy!

Other notable commands:

`make compile` -- recompiles the source files if you've altered the compiler.
