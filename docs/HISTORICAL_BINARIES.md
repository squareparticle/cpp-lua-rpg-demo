# Historical Binaries

Historical executables, debug symbols, and binary archives were intentionally excluded from the first Git candidate. They remain available in the separately backed-up original folder.

No executable was run or reverse engineered during the audit or cleanup. Relationships below come from filenames, sizes, timestamps, project records, and SHA-256 hashes.

## Executables

| Historical path | Size | Audit finding |
|---|---:|---|
| `Dragon Quest.exe` | 202,752 bytes | Byte-for-byte identical to root `hour7.exe`. |
| `hour7.exe` | 202,752 bytes | Byte-for-byte identical to `Dragon Quest.exe`. Both had SHA-256 `4754C7D4C7DB1A96C28A695B458423B3F88FD6D8408750C756554B7C1D7F27EE`. |
| `Debug\hour7.exe` | 202,752 bytes | Same size as the root pair but a different SHA-256; likely a distinct Debug build. |
| `Debug\New WinZip File\hour7.exe` | 198,656 bytes | Older, smaller executable preserved both as an extracted file and inside the ZIP archive. |
| `message.exe` | 168,448 bytes | Named historical variant; exact source/configuration relationship unknown. |
| `nosound.exe` | 167,936 bytes | Named historical variant; exact source/configuration relationship unknown. |
| `sounds.exe` | 168,448 bytes | Named historical variant; different hash from `message.exe` despite equal size. |

## Debug symbols and binary archives

- `Debug\hour7.pdb` was the preserved symbol database associated with a historical Debug output. It was excluded because PDBs may contain build-machine paths and are generated binary artifacts.
- `Debug\New WinZip File.zip` contained `Weather.BMP` and the older 198,656-byte `hour7.exe`. It was excluded from the Git candidate as a historical binary/media archive.
- `hour7.plg` is not a program binary. It was retained under `legacy/project/` because it is readable build evidence recording a successful zero-error historical Debug build.

## What can and cannot be concluded

The identical root executable pair strongly suggests that `Dragon Quest.exe` was a renamed or copied `hour7.exe`. The project and build log also identify `hour7.exe` as the output name. Timestamps alone cannot prove which distinct binary was the final intended release, because much of the folder carries migration/copy timestamps. Determining behavioral differences would require a separately authorized executable comparison.
