# Spec-Win32
Experimental native 64-bit Win32 backend for Spec/Pharo

## Loding

Copy `spec-win32-runtime.dll` next to your image or VM executable.

```smalltalk
Metacello new
  baseline: 'SpecWin32';
  repository: 'github://pavel-krivanek/Spec-Win32/src';
  load: #Full.
```

See `SpWin32Examples` class.
