#!/usr/bin/env python3
"""
XAPK 재패키징: 원본 XAPK 안의 베이스 APK 만 갤럭시 S23용
(targetSdkVersion=24) APK 로 교체한다. OBB 확장 파일은 그대로 보존한다.

배경: apkfab/APKPure 의 .xapk 는 ZIP 컨테이너로
  - 베이스 APK ( <pkg>.apk )
  - Android/obb/<pkg>/main.*.obb, patch.*.obb (확장 파일)
  - manifest.json (패키지/버전/파일 목록 메타데이터)
를 담는다. 원본 APK 는 targetSdkVersion=21 이라 Android 14/15 에서
설치가 차단되므로, 코드/네이티브가 동일하고 targetSdkVersion 만 24 로
올린 APK 로 교체하면 OBB 와 함께 그대로 설치/구동된다.

확장 파일(OBB)은 손대지 않으므로 게임 데이터/검증값은 그대로 일치한다.

사용법:
  python3 repack_xapk.py <원본.xapk> <교체APK.apk> <출력.xapk> [패키지명]
예:
  python3 repack_xapk.py "Order Up To Go_v1.65_apkfab.com.xapk" \\
      OrderUp-1.65-sdk24-signed.apk OrderUp-1.65-s23.xapk
"""
import hashlib
import json
import sys
import zipfile

DEFAULT_PKG = 'com.svsgames.orderup'


def pick_base_apk_entry(names, pkg):
    """XAPK 안에서 교체할 베이스 APK 엔트리 이름을 고른다."""
    apks = [n for n in names if n.lower().endswith('.apk')]
    if not apks:
        raise SystemExit('XAPK 안에서 .apk 파일을 찾지 못했습니다.')
    # 1) <pkg>.apk 정확히 일치
    for n in apks:
        if n.lower() in (f'{pkg}.apk', f'{pkg}/{pkg}.apk'):
            return n
    # 2) split 이 아닌(이름에 config/split 없는) 단일 base apk 우선
    base = [n for n in apks if 'split' not in n.lower() and 'config' not in n.lower()]
    cand = base or apks
    if len(cand) == 1:
        return cand[0]
    # 3) 여러 개면 가장 큰 .apk 를 base 로 간주 (호출 측에서 확인 권장)
    return None  # 모호 -> 호출부에서 처리


def update_manifest(raw, old_name, new_size, new_sha256):
    """manifest.json(JSON)에서 교체된 APK의 크기/체크섬/total_size 를 갱신.
    구조가 다양하므로 'file' 이 old_name 과 매칭되는 항목만 손본다."""
    try:
        data = json.loads(raw.decode('utf-8'))
    except Exception:
        return raw  # JSON 아니면 그대로 둠
    delta = [0]

    def fix_entry(e):
        if not isinstance(e, dict):
            return
        f = e.get('file') or e.get('name')
        if f and (f == old_name or f.endswith('/' + old_name) or f.endswith(old_name)):
            if 'size' in e and isinstance(e['size'], int):
                delta[0] += new_size - e['size']
                e['size'] = new_size
            for k in ('sha256', 'sha1', 'md5', 'hash', 'checksum'):
                if k in e:
                    e[k] = new_sha256 if k in ('sha256', 'hash', 'checksum') else e[k]

    # split_apks / expansions / 임의 리스트 순회
    for key in ('split_apks', 'apk', 'apks', 'expansions', 'files'):
        v = data.get(key)
        if isinstance(v, list):
            for e in v:
                fix_entry(e)
        elif isinstance(v, dict):
            fix_entry(v)
    # total_size 보정
    if isinstance(data.get('total_size'), int):
        data['total_size'] += delta[0]
    return json.dumps(data, ensure_ascii=False, indent=2).encode('utf-8')


def repack(src_xapk, new_apk, dst_xapk, pkg, base_override=None):
    new_bytes = open(new_apk, 'rb').read()
    new_size = len(new_bytes)
    new_sha = hashlib.sha256(new_bytes).hexdigest()

    with zipfile.ZipFile(src_xapk, 'r') as zin:
        names = zin.namelist()
        base = base_override or pick_base_apk_entry(names, pkg)
        if base is None:
            apks = [n for n in names if n.lower().endswith('.apk')]
            raise SystemExit(
                'base APK 를 자동 판별하지 못했습니다. 네 번째 인자로 base apk 엔트리명을 지정하세요.\n'
                f'후보: {apks}')
        print(f'[repack] base APK 엔트리: {base}')
        print(f'[repack] 새 APK 크기: {new_size:,} bytes, sha256={new_sha[:16]}...')

        with zipfile.ZipFile(dst_xapk, 'w') as zout:
            for info in zin.infolist():
                n = info.filename
                if n == base:
                    data = new_bytes
                    print(f'  replace {n}')
                elif n.lower().endswith('manifest.json') or n == 'manifest.json':
                    data = update_manifest(zin.read(n), base.split('/')[-1], new_size, new_sha)
                    print(f'  update  {n} (apk size/checksum/total_size 보정)')
                else:
                    data = zin.read(n)
                    if n.lower().endswith('.obb'):
                        print(f'  keep    {n} ({info.file_size:,} bytes)')
                ni = zipfile.ZipInfo(n, date_time=info.date_time)
                ni.compress_type = info.compress_type
                ni.external_attr = info.external_attr
                zout.writestr(ni, data)
    print(f'[repack] 완료: {dst_xapk}')


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)
    src, apk, dst = sys.argv[1], sys.argv[2], sys.argv[3]
    pkg = sys.argv[4] if len(sys.argv) > 4 and not sys.argv[4].lower().endswith('.apk') else DEFAULT_PKG
    base_override = sys.argv[5] if len(sys.argv) > 5 else None
    repack(src, apk, dst, pkg, base_override)


if __name__ == '__main__':
    main()
