"""Install the official pinned GBDK release locally (never changes system tools)."""
from pathlib import Path
import hashlib,platform,tarfile,urllib.request
VERSION='4.5.0'
ROOT=Path(__file__).resolve().parents[1]
choices={('Darwin','arm64'):'macos-arm64',('Darwin','x86_64'):'macos',('Linux','x86_64'):'linux64',('Linux','aarch64'):'linux-arm64'}
flavor=choices.get((platform.system(),platform.machine()))
if not flavor:raise SystemExit('Use the official GBDK 4.5.0 release for your platform and set GBDK_HOME.')
url=f'https://github.com/gbdk-2020/gbdk-2020/releases/download/{VERSION}/gbdk-{flavor}.tar.gz'
dest=ROOT/'.tools';dest.mkdir(exist_ok=True);archive=dest/f'gbdk-{flavor}.tar.gz'
print(f'Downloading official GBDK {VERSION}: {url}')
with urllib.request.urlopen(url) as r:archive.write_bytes(r.read())
digest=hashlib.sha256(archive.read_bytes()).hexdigest()
if flavor=='macos-arm64' and digest!='289ee60e46c5a2785a21e35533f84a5131ed4a063b21b0dbdedc9a10af15bf78':raise SystemExit('Archive checksum mismatch')
print(f'SHA256: {digest}')
with tarfile.open(archive) as t:t.extractall(dest,filter='data')
print(f'Installed to {dest / "gbdk"}')
