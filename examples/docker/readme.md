## How to use:

```
docker build -t socketman:latest ./
sudo mkdir /opt/socketman && sudo chown -R 1000:1000 /opt/socketman
docker run -e LEDE_TARGET=x86_64 -v /opt/socketman:/builddir/save socketman
```

Once done, file will be in your volume of the container. In the above example this would be located at /opt/socketman. You can also run without a volume mount, but then the file will be stuck in a docker volume until you extract it.

### Options:

- LEDE_TARGET = Target/Subtarget you want to build with. Examples are apm821xx, ar71xx, etc.
- LEDE_REPO = Repo of LEDE/OpenWRT you want to use. LEDE is used by default.
- LEDE_BRANCH = Branch of the repo you want to use. "master" is used by default.
