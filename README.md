# Warning: This is very old and not maintained. Do not use

# ccache-lipo
a ccache wrapper for apple universal builds

A universal build on macosx calls internally a compiler for each `-arch` you specify, and run `lipo` afterwards to combine the artifacts to one object file.

`ccache-lipo` does this manually by running `ccache` for each architecture separately and manually doing the `lipo` step afterwards.
