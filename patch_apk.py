#!/usr/bin/env python3
"""
Order Up!! (com.svsgames.orderup) APK 호환성 패치 스크립트.

목적: 2015년 빌드(targetSdkVersion=21) APK는 Android 14+ 에서
INSTALL_FAILED_DEPRECATED_SDK_VERSION 으로 설치가 차단된다.
AndroidManifest.xml 의 targetSdkVersion 정수만 새 값으로 바꾸고
(파일 크기/오프셋 불변), 구 서명(META-INF)을 제거하여 재패키징한다.
이후 zipalign + apksigner 로 재서명하면 갤럭시 S23(Android 14/15)에 설치 가능.

게임 로직/리소스/네이티브 라이브러리는 일절 건드리지 않는다.
"""
import struct
import sys
import zipfile

# Android 표준 attribute resource id
ATTR_TARGET_SDK = 0x01010270
ATTR_MIN_SDK = 0x0101020C
TYPE_INT_DEC = 0x10


def find_uses_sdk_attr_offset(axml: bytes, attr_res_id: int):
    """압축 해제된 바이너리 AndroidManifest.xml 에서 주어진 attribute 의
    typed-value(4바이트 정수) 데이터가 위치한 파일 오프셋과 현재 값을 반환."""
    def u16(o):
        return struct.unpack_from('<H', axml, o)[0]

    def u32(o):
        return struct.unpack_from('<I', axml, o)[0]

    assert u16(0) == 0x0003, 'AXML 매직이 아님'
    # string pool chunk @ offset 8
    off = 8
    assert u16(off) == 0x0001, 'string pool 청크 아님'
    sp_size = u32(off + 4)

    o = off + sp_size
    res_ids = []
    while o + 8 <= len(axml):
        ctype = u16(o)
        csize = u32(o + 4)
        if csize == 0:
            break
        if ctype == 0x0180:  # RESOURCE_MAP
            res_ids = [u32(o + 8 + i * 4) for i in range((csize - 8) // 4)]
        elif ctype == 0x0102:  # START_TAG
            attr_start = u16(o + 24)
            attr_size = u16(o + 26)
            attr_count = u16(o + 28)
            ap = o + 16 + attr_start
            for i in range(attr_count):
                a = ap + i * attr_size
                a_name = u32(a + 4)
                dtype = axml[a + 15]
                rid = res_ids[a_name] if a_name < len(res_ids) else 0
                if rid == attr_res_id:
                    return a + 16, u32(a + 16), dtype
        o += csize
    return None, None, None


def patch_manifest(axml: bytes, new_target: int) -> bytes:
    doff, cur, dtype = find_uses_sdk_attr_offset(axml, ATTR_TARGET_SDK)
    if doff is None:
        raise RuntimeError('targetSdkVersion attribute 를 찾지 못했습니다.')
    if dtype != TYPE_INT_DEC:
        raise RuntimeError(f'예상치 못한 dataType={hex(dtype)} (정수 아님).')
    print(f'  targetSdkVersion: {cur} -> {new_target} (offset {doff})')
    return axml[:doff] + struct.pack('<I', new_target) + axml[doff + 4:]


def repackage(src_apk: str, dst_apk: str, new_target: int):
    with zipfile.ZipFile(src_apk, 'r') as zin:
        infos = zin.infolist()
        manifest = zin.read('AndroidManifest.xml')
        patched = patch_manifest(manifest, new_target)

        with zipfile.ZipFile(dst_apk, 'w') as zout:
            for info in infos:
                name = info.filename
                # 구 서명 및 잔여 META-INF 항목 전부 제외 (재서명할 것이므로)
                if name.startswith('META-INF/'):
                    print(f'  drop  {name}')
                    continue
                data = patched if name == 'AndroidManifest.xml' else zin.read(name)
                # 원본 압축 방식 보존 (resources.arsc 는 STORED 여야 함)
                ni = zipfile.ZipInfo(name, date_time=info.date_time)
                ni.compress_type = info.compress_type
                ni.external_attr = info.external_attr
                ni.create_system = info.create_system
                zout.writestr(ni, data)
    print(f'  wrote {dst_apk}')


def main():
    if len(sys.argv) < 3:
        print('usage: patch_apk.py <in.apk> <out_unsigned.apk> [target_sdk=24]')
        sys.exit(1)
    src, dst = sys.argv[1], sys.argv[2]
    target = int(sys.argv[3]) if len(sys.argv) > 3 else 24
    print(f'[patch] {src} -> {dst} (targetSdkVersion={target})')
    repackage(src, dst, target)


if __name__ == '__main__':
    main()
