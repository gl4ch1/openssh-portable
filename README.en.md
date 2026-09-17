# GLSSH — an OpenSSH fork

This repository is a fork of [OpenSSH](https://www.openssh.com) ([openssh-portable](https://github.com/openssh/openssh-portable)), based on version **OpenSSH_10.2** (see [version.h](version.h)).

The added functionality is described below. Everything else is standard portable OpenSSH: the build process, documentation and license are unchanged (see [LICENCE](LICENCE) and [CREDITS](CREDITS) — this fork is distributed under the same license as the original OpenSSH).

---

### Description

GLSSH is an OpenSSH extension that intercepts authentication prompts (password, OTP, passphrase) and automatically substitutes a response based on configured events.

### Features

* Intercepting any input prompt (password, OTP, PIN, passphrase)
* Matching the prompt against a regular expression (POSIX regex)
* Substituting static text
* Running commands and scripts to generate a response dynamically
* Multiple events with usage-count control

### Additionally

* Existing standard parameters can be extended or overridden

### Configuration syntax

```
GLSSHAddRequestEvent "regex" "data" [count]
```

| Parameter | Description |
| --- | --- |
| `regex` | Regular expression to match against the prompt string |
| `data` | Text to substitute, or a command (`$(script or path to script)`) |
| `count` | Number of times the event may be used (default 1) |

### Examples

```
# ~/.ssh/config

# Substitute a static password
GLSSHAddRequestEvent "[Pp]assword" "mysecret"

# Run a command for OTP
GLSSHAddRequestEvent "OTP:" "$(oathtool --totp -b SECRET)"

# Complex regex
GLSSHAddRequestEvent "^\(user@host\).*Password: $" "$(pass show host/password)"

# Multiple events (executed in order)
GLSSHAddRequestEvent "Username:" "admin"
GLSSHAddRequestEvent "Password:" "$(/usr/local/bin/get-pass.sh)"
GLSSHAddRequestEvent "OTP:" "$(oathtool --totp -b pass show totp/secret)"

# Overriding standard parameter values
Host test-*
    User user1

Host test-2
    User user2

# Extending standard parameter values
Host test-*
    User user

Host test-2
    User +2
```

### Usage-count control

```
# The event can be used 3 times
GLSSHAddRequestEvent "Password:" "mysecret" 3

# Retries: the event fires 5 times
GLSSHAddRequestEvent "[P-p]assword" "temp123" 5
```

### Positional substitution for keyboard-interactive

When the server sends several prompts in a row with identical or empty text, you can rely on event order and usage count. For this, the `.*` regex is used, which matches any prompt:

```
# First prompt (any) → username
GLSSHAddRequestEvent ".*" "admin"
# Second prompt → password
GLSSHAddRequestEvent ".*" "$(pass show host/password)"
# Third prompt → OTP
GLSSHAddRequestEvent ".*" "$(oathtool --totp -b SECRET)"
```

Events are applied in order: each one fires exactly once (or the specified number of times), then moves on to the next. This makes it possible to automate dialogs where the prompt text is indistinguishable or absent.

### How it works

1. The server requests authentication (e.g. `"Password: "`)
2. The client intercepts the prompt in `read_passphrase()`
3. It checks all events in the order they were added
4. It finds the first event whose regex matches the prompt
5. It substitutes `data` as the response (if `data` starts with `$`, it runs the command)
6. The user gets access without typing anything manually

### Debugging

```
ssh -v host 2>&1 | grep GLSSH
```

### License

GLSSH is an extension of OpenSSH and is distributed under the same license.

---

# Portable OpenSSH

[![C/C++ CI](../../actions/workflows/c-cpp.yml/badge.svg)](../../actions/workflows/c-cpp.yml)
[![VM CI](../../actions/workflows/vm.yml/badge.svg)](../../actions/workflows/vm.yml)
[![C/C++ CI self-hosted](https://github.com/openssh/openssh-portable-selfhosted/actions/workflows/selfhosted.yml/badge.svg)](https://github.com/openssh/openssh-portable-selfhosted/actions/workflows/selfhosted.yml)
[![CIFuzz](../../actions/workflows/cifuzz.yml/badge.svg)](../../actions/workflows/cifuzz.yml)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/openssh.svg)](https://issues.oss-fuzz.com/issues?q="Project:+openssh"+is:open)
[![Coverity Status](https://scan.coverity.com/projects/21341/badge.svg)](https://scan.coverity.com/projects/openssh-portable)

OpenSSH is a complete implementation of the SSH protocol (version 2) for secure remote login, command execution and file transfer. It includes a client ``ssh`` and server ``sshd``, file transfer utilities ``scp`` and ``sftp`` as well as tools for key generation (``ssh-keygen``), run-time key storage (``ssh-agent``) and a number of supporting programs.

This is a port of OpenBSD's [OpenSSH](https://openssh.com) to most Unix-like operating systems, including Linux, OS X and Cygwin. Portable OpenSSH polyfills OpenBSD APIs that are not available elsewhere, adds sshd sandboxing for more operating systems and includes support for OS-native authentication and auditing (e.g. using PAM).

**Note:** this fork (GLSSH) adds the GLSSH extension described above; everything below is unmodified upstream documentation.

## Documentation

The official documentation for OpenSSH are the man pages for each tool:

* [ssh(1)](https://man.openbsd.org/ssh.1)
* [sshd(8)](https://man.openbsd.org/sshd.8)
* [ssh-keygen(1)](https://man.openbsd.org/ssh-keygen.1)
* [ssh-agent(1)](https://man.openbsd.org/ssh-agent.1)
* [scp(1)](https://man.openbsd.org/scp.1)
* [sftp(1)](https://man.openbsd.org/sftp.1)
* [ssh-keyscan(8)](https://man.openbsd.org/ssh-keyscan.8)
* [sftp-server(8)](https://man.openbsd.org/sftp-server.8)

## Stable Releases

Stable release tarballs are available from a number of [download mirrors](https://www.openssh.com/portable.html#downloads). We recommend the use of a stable release for most users. Please read the [release notes](https://www.openssh.com/releasenotes.html) for details of recent changes and potential incompatibilities.

## Building Portable OpenSSH

### Dependencies

Portable OpenSSH is built using autoconf and make. It requires a working C compiler, standard library and headers.

``libcrypto`` from one of [LibreSSL](https://www.libressl.org/), [OpenSSL](https://www.openssl.org), [AWS-LC](https://github.com/aws/aws-lc) or [BoringSSL](https://github.com/google/boringssl) may also be used. OpenSSH may be built without either of these, but the resulting binaries will have only a subset of the cryptographic algorithms normally available.

[zlib](https://www.zlib.net/) is optional; without it transport compression is not supported.

FIDO security token support needs [libfido2](https://github.com/Yubico/libfido2) and its dependencies and will be enabled automatically if they are found.

In addition, certain platforms and build-time options may require additional dependencies; see README.platform for details about your platform.

### Building a release

Release tarballs and release branches in git include a pre-built copy of the ``configure`` script and may be built using:

```
tar zxvf openssh-X.YpZ.tar.gz
cd openssh
./configure # [options]
make && make tests
```

See the [Build-time Customisation](#build-time-customisation) section below for configure options. If you plan on installing OpenSSH to your system, then you will usually want to specify destination paths.

### Building from git

If building from the git master branch, you'll need [autoconf](https://www.gnu.org/software/autoconf/) installed to build the ``configure`` script. The following commands will check out and build portable OpenSSH from git:

```
git clone https://github.com/openssh/openssh-portable # or https://anongit.mindrot.org/openssh.git
cd openssh-portable
autoreconf
./configure
make && make tests
```

### Build-time Customisation

There are many build-time customisation options available. All Autoconf destination path flags (e.g. ``--prefix``) are supported (and are usually required if you want to install OpenSSH).

For a full list of available flags, run ``./configure --help`` but a few of the more frequently-used ones are described below. Some of these flags will require additional libraries and/or headers be installed.

Flag | Meaning
--- | ---
``--with-pam`` | Enable [PAM](https://en.wikipedia.org/wiki/Pluggable_authentication_module) support. [OpenPAM](https://www.openpam.org/), [Linux PAM](http://www.linux-pam.org/) and Solaris PAM are supported.
``--with-libedit`` | Enable [libedit](https://www.thrysoee.dk/editline/) support for sftp.
``--with-kerberos5`` | Enable Kerberos/GSSAPI support. Both [Heimdal](https://www.h5l.org/) and [MIT](https://web.mit.edu/kerberos/) Kerberos implementations are supported.
``--with-selinux`` | Enable [SELinux](https://en.wikipedia.org/wiki/Security-Enhanced_Linux) support.

## Development

Portable OpenSSH development is discussed on the [openssh-unix-dev mailing list](https://lists.mindrot.org/mailman/listinfo/openssh-unix-dev) ([archive mirror](https://marc.info/?l=openssh-unix-dev)). Bugs and feature requests are tracked on our [Bugzilla](https://bugzilla.mindrot.org/).

## Reporting bugs

_Non-security_ bugs may be reported to the developers via [Bugzilla](https://bugzilla.mindrot.org/) or via the mailing list above. Security bugs should be reported to [openssh@openssh.com](mailto:openssh.openssh.com).
