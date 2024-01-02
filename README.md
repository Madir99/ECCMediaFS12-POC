# ECCMediaFS12-POC

ECC Media Filesystem (EMFS) is a filesystem with an ECC feature based on Extended Hamming (511, 502) code. This 12-bit version is specifically designed for floppy disks. Please note that this project is a proof of concept (POC), and its future may involve incorporation into another project.

---

## TODO

- [ ] Add support for directories
- [ ] Develop a filesystem checker (utilizing ECC for checking and fixing)
- [ ] Implement variable node size (currently fixed at 512 bytes)
- [ ] Implement compression (Consider using a custom algorithm; if a custom algorithm is chosen, develop it as a separate GitHub project)
- [ ] General code & struct cleanup (May break already formated drives)
- [ ] Create a Linux driver (to be developed as a separate GitHub project)


---

## Usage

This POC is targeted to manipulation with raw image files. In case of Linux you can actually use it on devices like /dev/sd* or /dev/fd*.
For file system (FS) manipulation, use the following command (with `read` used as an example):
```bash
./read IMAGE FILE
```
For FS creation, the syntax is as follows:
```bash
./mkfs IMAGE [FS_LABEL] [FS_SIZE_IN_NODES] [CUSTOM_BOOT_CODE]
```

---

## Compare to FAT12

This filesystem, ECCMediaFS12 (EMFS12), offers several distinctions from the traditional FAT12 file system. Below is a comparison table:

| Feature                         | FAT12                   | EMFS12               |
| ------------------------------- | ----------------------- | -------------------- |
| Compression                     | ❌                      | ❌ (Not added yet)    |
| ECC                             | ❌                      | ✔️                    |
| File dates & times              | ✔️                      | ❌                    |
| Long File Names support         | ✔️                      | ❌                    |
| Variable cluster/node size      | ✔️                      | ❌ (Not added yet)    |
| Volume boot record              | ✔️                      | ✔️                    |
| Directory size                  | fixed to fit in cluster | fixed to fit in node |
| Root directory size             | 224*                    | fixed to fit in node |
| Free Space                      | 1 457 664 bytes         | 1 439 000 bytes      |
| Directory entries per 512 bytes | 16 entries              | 28 entries           |
| Filesystem Type                 | File Allocation Table   | Node chain           |

**Notes:**
- Both file systems were tested on media with a size of 1 474 560 bytes (floppy disk).
- FAT12 used in this comparison was formatted with a cluster size of 512 bytes, and other values were kept in default.
- *Value can be modified during formatting.

**Conclusion:**
EMFS12 introduces ECC support compared to FAT12. While FAT12 supports features such as file dates & times, long file names support, and variable cluster sizes (not yet implemented in EMFS12), EMFS12 focuses on data integrity.

---

## License

This project is licensed under the [GNU AGPLv3](LICENSE).

---

## Acknowledgments

- Special thanks to [ChatGPT](https://www.openai.com/) for helping me with creating this README.md.
- With updates that break already formated drives may or may not came out FS conversion program.
- I am taking break from development generaly for while because I have to do stuff to school.
