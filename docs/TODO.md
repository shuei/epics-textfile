TODO list for textfile support
====

- [x] Add ai record support.
  - [x] Support NaN / Inf.
  - [ ] Support IEEE754 binary64 format and binary32 format.
- [x] Add longin record support.
  - [x] Support hexadecimal values.
- [ ] Add bi record support.
- [x] Add stringin record support.
- [ ] Add lsi record support.
- [x] Add waveform record support.
  - [x] Support NaN / Inf.
  - [x] Support hexadecimal values.
  - [ ] Support IEEE754 binary64 format and binary32 format.
  - [x] Check if FTVL is valid within init_record().
- [x] Output record support which writes to file.
  - [x] longout record support.
  - [x] ao record support.
- [x] Add option to read from file during iocInit().
- [ ] Add option to keep input file opened rather than re-open on every process.
  - [ ] Check if i-node number has been changed associated to the file name.
- [ ] Consider making the device support asynchronous.
