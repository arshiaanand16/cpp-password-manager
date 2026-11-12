# C++ Password Manager

A command-line password manager I built in C++ during my festive break. It keeps your passwords in an encrypted file and asks for a master password before letting you in.

This is a learning project, so please don't use it for your real passwords.

# What it can do
- Set a master password the first time you run it, and ask for it every time after (3 attempts)
- Hide the master password while you type it
- Add, view, search, update and delete entries
- Stop you from saving the same site twice

# How the security works
I wanted to understand how this is done properly, so I used libsodium and didn't write any crypto myself.

- **Login:** the master password is never saved. Only a salted Argon2id hash of it goes into `master.dat`, and when you log in the program hashes what you typed and compares.
- **Encryption key:** the key that locks the vault comes from your master password plus a separate random salt (`salt.dat`). It's never written to disk. I used a different salt from the login hash on purpose, so the stored hash can't be used to open the vault.
- **The vault:** entries are encrypted with libsodium's `crypto_secretbox_easy`, using a fresh random nonce every time the file is saved.
- **Tampering:** if even one byte of `vault.dat` is changed, decryption fails and the program refuses to open it. I tested this by editing the file by hand.

# Files it creates
| File | What's in it |
|------|--------------|
| `master.dat` | Hash of the master password |
| `salt.dat` | Salt used to make the encryption key |
| `vault.dat` | Your encrypted entries |

All three are in `.gitignore`, so they never get uploaded.

# How to run it
You need a C++ compiler and libsodium. On Windows I used MSYS2 (UCRT64):

pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libsodium
g++ password_manager.cpp -o password_manager -lsodium
./password_manager


# Known limitations
- Windows only, because the hidden input uses `windows.h`
- Site names and passwords can't contain spaces
- If you forget the master password, there's no way to recover your data
- Decrypted entries sit in memory while the program is running
- The 3-attempt limit resets if you restart the program
- Passwords are printed on screen when you view or search

# What I learned
- Why passwords get hashed with a salt instead of being stored as they are
- Hashing and encryption are different things: one is one-way, the other can be reversed with a key
- Why you should use a trusted library instead of writing your own crypto
- Key derivation, nonces, and authenticated encryption
- How to use Git and GitHub, including keeping secrets and compiled files out of a repo

# What I'd like to add next
- Hidden input that works on every operating system
- Support for spaces in entries
- Copying a password to the clipboard instead of printing it