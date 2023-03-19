## Chapter 9: Storage Management

### The Volume Namespace

#### Mount Points

- Mount points let you link volumes through directories on NTFS volumes, which makes volumes with no drive-letter assignment accessible.
- What makes mount points possible is __reparse point__ technology.
- A reparse point is a block of arbitrary data with some fixed header data that Windows associates with an NTFS file or directory.
- One common use of reparse points is the symbolic link functionality offered on Windows by NTFS.
- Mount points are reparse points that store a volume name `\Global??\Volume{X}` as the reparse data.
