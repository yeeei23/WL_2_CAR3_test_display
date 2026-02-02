# MP3 → PWM0 출력 (mp3_to_pwm.py)

특정 MP3 파일을 디코딩해 PCM으로 변환한 뒤, **PWM0** 핀의 듀티비로 출력하는 방식입니다.

## 동작 방식

1. **mpg123** (우선) 또는 **ffmpeg**로 MP3 → raw PCM (s16le, 모노, 44.1kHz)
2. Linux **sysfs PWM** (`/sys/class/pwm/pwmchip0/pwm0`) 에서:
   - `period`: 고정 (예: 1000 ns = 1 MHz 캐리어)
   - `duty_cycle`: 오디오 샘플 값에 비례해 매 샘플마다 갱신
3. PWM0 핀 출력을 **RC 저역통과 필터**로 걸면 아날로그 오디오로 복원 가능

## 사용법

```bash
# 기본 (PWM0 = pwmchip0 채널 0)
python3 mp3_to_pwm.py assets/music.mp3

# PWM 칩/채널 지정 (PWM0이 다른 칩이면)
python3 mp3_to_pwm.py assets/music.mp3 --pwm-chip 0 --pwm-channel 0

# 샘플레이트 변경 (기본 44100)
python3 mp3_to_pwm.py assets/music.mp3 --sample-rate 48000

# PWM period가 Invalid argument면 보드 최소값이 더 큼 → period 늘려서 시도 (기본 10000ns=100kHz)
python3 mp3_to_pwm.py assets/music.mp3 --pwm-period 100000
```

종료: **Ctrl+C**

## 요구사항

- **Python 3**
- **mpg123** 또는 **ffmpeg** (MP3 디코딩, mpg123이 있으면 mpg123 사용. Yocto 등에서는 mpg123만 넣어도 동작)
- **PWM sysfs** 사용 가능한 Linux (예: 라즈베리파이, BeagleBone, PWM 지원 SoC)
- PWM 디렉터리 쓰기 권한 (보통 root 또는 `pwm` 그룹)

## PWM0 경로

- 기본: `/sys/class/pwm/pwmchip0/pwm0`
- 보드에 따라 `pwmchipN` 번호가 다를 수 있음. 다음으로 확인:

```bash
ls /sys/class/pwm/
# pwmchip0, pwmchip1, ... 중 사용할 칩 선택
echo 0 | sudo tee /sys/class/pwm/pwmchip0/export   # pwm0 생성
ls /sys/class/pwm/pwmchip0/pwm0/
# period, duty_cycle, enable 등
```

## 하드웨어 (오디오 복원)

PWM0 핀만으로는 디지털 구형파이므로, **RC 저역통과 필터**를 거치면 아날로그 오디오에 가깝게 쓸 수 있습니다.

- PWM 캐리어: 1 MHz (period 1000 ns)
- 권장: R ≈ 1kΩ, C ≈ 100nF ~ 1µF (차단 주파수 약 1.6kHz ~ 160Hz)
- 필터 출력 → 앰프/스피커 또는 ELB080306 등 모듈 입력

## QML UI

`test_spk.qml` 은 **일반 재생**(Qt MediaPlayer)용입니다.  
**PWM0 재생**은 위처럼 터미널에서 `mp3_to_pwm.py` 를 실행하면 됩니다.
