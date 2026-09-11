# Darling

![Darling logo](https://darlinghq.org/img/darling250.png)

**Darling** is a runtime environment for macOS applications on Linux.

[GitHub](https://github.com/darlinghq/darling) · [Documentation](https://docs.darlinghq.org) · [Website](https://darlinghq.org)

---

## !!! Requirements

Darling requires the `darling-mach` kernel module loaded on the **host** machine.  
This image alone is not sufficient — see [build instructions](https://docs.darlinghq.org/build-instructions.html) to set up the kernel module.

## Usage

```bash
docker run --rm --privileged \
  darlinghq/darling darling shell echo "Hello from Darling"
```

## Tags

| Tag | Description |
|-----|-------------|
| `latest` | Latest build from `master` |
| `vX.Y.Z` | Specific release |
| `git-XXXXXXX` | Specific commit |

## Building locally

```bash
git clone --recursive https://github.com/darlinghq/darling.git
cd darling
docker build -f ci/Darling.Dockerfile -t darling .
```
