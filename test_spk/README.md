# test_spk

트리거 신호 → 소리 재생 + Qt UI 갱신 프로젝트.

## 빌드

```bash
./build.sh
```

결과물: `output/` 폴더 (key_trigger, sound_trigger, test_spk_host)

## 문서

- **docs/BUILD_FILES.md** — 빌드에 포함되는 소스/헤더 목록
- **docs/README_ARCHITECTURE.md** — 전체 구성·실행 순서
- **docs/README_TRIGGER.md** — 빌드·실행 방법
- **docs/README_BUILD_ELSEWHERE.md** — 다른 환경에서 빌드 후 복사
- **docs/README_WSL_BUILD.md** — WSL에서 빌드·보드 배포
- **docs/README_SCP_WSL.md** — scp로 WSL에 파일 보내기
- **docs/README_PWM.md** — MP3 → PWM 출력
- **docs/trigger_send.md** — trigger_send 모듈 사용법
- **docs/USAGE_FROM_OTHER_PROGRAMS.md** — 다른 프로그램에서 QML/소리 호출 방법

## 유틸

- **scripts/list_specs.sh** — qmake 사용 가능 스펙 목록 (host 빌드 시 -spec 확인용)
