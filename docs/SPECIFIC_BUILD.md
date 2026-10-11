<details>
<summary>Debian / Ubuntu / Linux Mint</summary>
<pre><code>sudo apt update
sudo apt install libx11-dev libxcursor-dev libxinerama-dev build-essential</code></pre>
</details>

<details>
<summary>Arch Linux / Manjaro</summary>
<pre><code>sudo pacman -Syy
sudo pacman -S libx11 libxinerama gcc make</code></pre>
</details>

<details>
<summary>Gentoo</summary>
<pre><code>sudo emerge --ask x11-libs/libX11 x11-libs/libXinerama sys-devel/gcc sys-devel/make
sudo emaint -a sync
</code></pre>
</details>

<details>
<summary>Void Linux</summary>
<pre><code>sudo xbps-install -S
sudo xbps-install libX11-devel libXinerama-devel libXcursor-devel gcc make</code></pre>
</details>

<details>
<summary>Fedora / RHEL / AlmaLinux / Rocky</summary>
<pre><code>sudo dnf update
sudo dnf install libX11-devel libXcursor-devel libXinerama-devel gcc make</code></pre>
</details>

<details>
<summary>OpenSUSE (Leap / Tumbleweed)</summary>
<pre><code>sudo zypper refresh
sudo zypper install libX11-devel libXinerama-devel gcc make</code></pre>
</details>

<details>
<summary>Alpine Linux</summary>
<pre><code>doas apk update
doas apk add libx11-dev libxinerama-dev libxcursor-dev gcc make musl-dev linux-headers</code></pre>
</details>

<details>
<summary>NixOS</summary>
<pre><code>buildInputs = [
  pkgs.xorg.libX11
  pkgs.xorg.libXinerama
  pkgs.libgcc
  pkgs.gnumake
];
sudo nixos-rebuild switch
</code></pre>
</details>

<details>
<summary>Slackware</summary>
<pre><code>slackpkg update
slackpkg install gcc make libX11 libXinerama</code></pre>
</details>

<details>
<summary>OpenBSD</summary>
<pre><code>doas pkg_add gmake</code></pre>
You will also need the X sets (<code>xbase</code>, <code>xfonts</code>, <code>xserv</code> and <code>xshare</code>) installed.
When you make the code, use <code>gmake</code> instead of <code>make</code> (which will be BSD make). Use the following command to build: <code>gmake CFLAGS="-I/usr/X11R6/include -Wall -Wextra -O3 -Isrc" LDFLAGS="-L/usr/X11R6/lib -lX11 -lXinerama -lXcursor"</code>
</details>

<details>
<summary>FreeBSD</summary>
<pre><code># If you use doas or su instead of sudo, modify the following commands accordingly.
sudo pkg update
sudo pkg install gcc gmake libX11 libXinerama</code></pre>
</details>

<details>
<summary>Termux</summary>
<pre><code>pkg install x11-repo
pkg update
pkg install clang make xcb-util-keysyms xorgproto libxcursor libx11 libxinerama libandroid-wordexp
# add `LDFLAGS="${LDFLAGS} -landroid-wordexp"` in the make command
</code></pre>
</details>

