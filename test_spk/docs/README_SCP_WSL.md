# scp로 WSL에 파일 보내기

파일을 **WSL로** 넣는 방법은 두 가지입니다.

---

## 1. WSL에서 끌어오기 (권장)

**빌드한 쪽(원격/다른 PC)** 에서 보내는 게 아니라, **WSL 안에서** scp로 원격에서 **가져오기(pull)** 하는 방식입니다.  
WSL에는 보통 `openssh-client`가 있어서 `scp`를 그대로 쓸 수 있습니다.

### 1-1. WSL 터미널에서 실행

```bash
# WSL 쪽 test_spk 경로로 이동 (예: /home/topst/test_spk)
cd /home/topst/test_spk

# 빌드 서버(또는 다른 PC)에서 host 실행 파일 가져오기
scp 사용자@빌드서버IP:/빌드서버에서/test_spk_host/경로/test_spk_host ./host/

# key_trigger 실행 파일 가져오기
scp 사용자@빌드서버IP:/빌드서버에서/key_trigger/경로/key_trigger ./key_trigger/

# 실행 권한 부여
chmod +x ./host/test_spk_host ./key_trigger/key_trigger
```

### 1-2. 예시

빌드 서버 IP가 `192.168.1.100`, 사용자 `builduser`,  
실행 파일이 각각 `/home/builduser/out/test_spk_host`, `/home/builduser/out/key_trigger` 에 있다면:

```bash
cd /home/topst/test_spk
scp builduser@192.168.1.100:/home/builduser/out/test_spk_host ./host/
scp builduser@192.168.1.100:/home/builduser/out/key_trigger ./key_trigger/
chmod +x ./host/test_spk_host ./key_trigger/key_trigger
```

이렇게 하면 **WSL 쪽으로 바로** 파일이 복사됩니다.

---

## 2. 원격에서 WSL로 밀어넣기(push)

**빌드한 쪽**에서 `scp 파일 wsl사용자@WindowsIP:경로` 처럼 **WSL 안으로** 보내려면, **WSL 안에서 SSH 서버**가 떠 있어야 하고, **Windows에서 WSL로 포트 포워딩**을 해 줘야 할 수 있습니다.

### 2-1. WSL에서 SSH 서버 설치·실행 (한 번만)

```bash
# Ubuntu/Debian WSL 예시
sudo apt update
sudo apt install openssh-server
sudo service ssh start
```

부팅할 때마다 켜려면:

```bash
sudo systemctl enable ssh
sudo systemctl start ssh
```

(WSL 기본 설정에선 `systemctl`이 동작하지 않을 수 있으므로, 필요하면 `sudo service ssh start`를 수동으로 실행하거나, Windows 시작 시 스크립트로 실행하는 방법을 쓰면 됩니다.)

### 2-2. WSL IP 확인

```bash
hostname -I
# 또는
ip addr show eth0
```

예: `172.20.10.5` 같은 주소가 나옵니다.  
이 주소는 **같은 Windows PC 안에서만** 유효하고, 다른 기기에서는 보통 **Windows IP**로 접속해야 합니다.

### 2-3. 같은 PC(Windows) 안에서만 push 하는 경우

빌드를 **Windows에서** 하고, **WSL로만** 보낼 때는 WSL IP로 바로 보낼 수 있습니다.

```bash
# Windows CMD/PowerShell이 아니라, WSL 터미널에서
# WSL IP 확인 (위에서 확인한 172.x.x.x)
scp /mnt/c/어디선가/test_spk_host topst@172.20.10.5:/home/topst/test_spk/host/
```

(실제로는 보통 WSL에서 `scp`로 **끌어오기**가 더 단순합니다.)

### 2-4. 다른 PC(빌드 서버)에서 WSL로 push 하는 경우

- 다른 PC에서는 보통 **WSL의 IP(172.x.x.x)**가 보이지 않습니다.
- 그래서 **Windows IP**로 접속하게 되고, Windows에 SSH 서버를 쓰거나, **Windows에서 WSL 22번 포트로 포워딩**한 뒤 `scp -P 포트 ...` 로 접속하는 식으로 설정해야 합니다.

요약하면:

- **다른 PC → WSL로 push**:  
  Windows에서 WSL SSH(22번)로 포트 포워딩 + (선택) Windows SSH 또는 WSL SSH 사용.  
  설정이 꽤 번거로우므로, **WSL에서 scp로 pull** 하는 방식을 쓰는 게 낫습니다.

---

## 정리

| 하고 싶은 일 | 추천 방법 |
|--------------|-----------|
| 빌드 서버/다른 PC → WSL로 파일 가져오기 | **WSL 터미널에서** `scp 사용자@서버:경로 ./host/` 처럼 **pull** |
| 같은 Windows PC 안에서만 WSL로 넣기 | WSL에 SSH 띄우고 `scp ... topst@WSL_IP:경로` 로 push 가능 |

**실무에서는**  
빌드 서버에서 파일을 만들고, **WSL에서 `scp 사용자@빌드서버:원격경로 ./host/`** 로 가져오는 방식이 가장 단순하고, "파일을 scp로 바로 WSL로 보낸다"는 목적도 충족합니다.
