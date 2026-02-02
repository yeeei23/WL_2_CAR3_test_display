# Yocto에서 전체화면(상단바 숨기기)

Yocto 이미지에서 Qt 앱을 전체화면으로 띄울 때 Weston 상단바가 보이는 경우 아래 방법 중 하나를 사용하세요.

## 1. Weston 없이 실행 (권장: 키오스크/클러스터)

Weston 데스크톱을 거치지 않고 Qt만 전체화면으로 띄우면 상단바가 없습니다.

```bash
export QT_QPA_PLATFORM=eglfs
./test_spk_host
```

또는:

```bash
export QT_QPA_PLATFORM=linuxfb
./test_spk_host
```

- **eglfs**: OpenGL ES 풀스크린. GPU 있는 보드에 적합.
- **linuxfb**: 프레임버퍼 직접 사용. GPU 없거나 단순한 보드에 적합.

이미지에 `eglfs`/`linuxfb` 플러그인과 의존 라이브러리가 포함되어 있어야 합니다.

## 2. Weston 쓰면서 상단바만 숨기기

Weston을 계속 쓰고 싶다면, Weston 설정에서 패널을 끕니다.

`/etc/xdg/weston/weston.ini` (또는 이미지에서 사용하는 weston.ini 경로)에 다음을 추가/수정:

```ini
[shell]
panel-position=none
panel-location=none
```

배경을 검정으로 하려면:

```ini
[core]
background-type=solid-color
background-color=0x000000ff
```

수정 후 Weston(또는 부팅)을 다시 시작해야 적용됩니다.

## 3. 앱 측에서 하는 것

호스트(`main.cpp`)에서는 이미 다음을 적용해 두었습니다.

- `Qt::FramelessWindowHint`: 앱 창 타이틀바 없음
- `Qt::WindowStaysOnTopHint`: 다른 창 위에 유지
- `setGeometry(screen->geometry())` 후 `showFullScreen()`: 화면 전체 사용

상단바 자체는 **컴포지터(Weston) 또는 QPA 플랫폼**이 결정하므로, 위 1번 또는 2번 설정이 필요합니다.
