Device Support for Text File
============================

# Overview

This is a device support for reading/writing values from/to text file. Following record types are supported for reading:
- longin
- ai
- stringin
- waveform

And following record type is supported for writing:
- longout

# Device type (DTYP) field
In order to use devTextFile, device type (DTYP) field must be set to "Text File" in the record:

`field(DTYP, "Text File")`

# Input/output link (INP/OUT) fields

General format for input (INP) and output (OUT) link fields are as following:

`field(INP, "@[<]filename"`

where `filename` is absolute or relative path to the file for reading/writing values. If the filename is prefixed with '<', initial values are read from the file during iocInit (even if it is an outout record).


# Input records

Each time the record is processed the device support opens the file specified in the INP field, reads the data from, and closes it.

An example db file of reading five elements of waveform is shown below:

```
record(waveform, "TEST:WAVEFORM:LONG") {
    field(SCAN, "Passive")
    field(DTYP, "Text File")
    field(PINI, "YES")
    field(INP,  "@/relative/or/absolute/path/to/input_file")
    field(NELM, "5")
    field(FTVL, "LONG")
}
```

where the `input_file` looks like:

```
# lines staring with '#', ';', '!', are ignred, empty lines as well.
# Describe one element of the waveform per line. Leading and trailing white spaces and tabs are ignored.

 1
 2
 3
 4
 5
```

# Output records

Each time the record is processed the device support opens the file specified in the OUT field, writes the data to, and closes it.

An example db file of writing 32 bit integer is shown below:

```
record(longout, "TEST:LONGOUT") {
    field(SCAN, "Passive")
    field(DTYP, "Text File")
    field(INP,  "@/relative/or/absolute/path/to/output_file")
    field(OOPT, "On Change")
}
```

where the `output_file` looks like:

```
# saved by devTextFileLo on example-ioc
# TEST:LONGOUT as of 2025-02-20 16:46:29.362471 (Thu)
1234
```
