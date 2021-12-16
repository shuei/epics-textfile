Device Support for Text File
============================

# Overview

This is a device support for reading values from text file. Following record types are supported:
- longin
- ai
- stringin
- waveform

# Example

This is an example db file which reads waveform of 5-elements from a text file:

```
record(waveform, "TEST:WAVEFORM:LONG") {
    field(SCAN, "Passive")
    field(DTYP, "Text File")
    field(PINI, "YES")
    field(INP,  "@/relative/or/absolute/path/to/file")
    field(NELM, "5")
    field(FTVL, "LONG")
}
```

where the text file looks like following:

```
# lines staring with '#', ';', '!', are ignred, empty lines as well.
# Describe one element of the waveform per line. Leading and trailing white spaces and tabs are ignored.

 1
 2
 3
 4
 5
```

Every time the record is processed, the file is (re-)opened and its content will be put into the record.
