# Elevator Control System

STM32G474와 FS90R 연속회전 서보모터로 구현한 5층 엘리베이터 제어 시스템이다.
초기 단일 `main.c` 중심 구현을 기능별 모듈과 FSM 구조로 정리하고, 진행 방향을 고려한 SCAN 방식 호출 처리, Hall Sensor 기반 층 감지, 오픈루프 가감속, 안전 상태 처리를 적용했다.

## 데모

<img width="1242" height="870" alt="5-floor elevator prototype" src="https://github.com/user-attachments/assets/8fa15b45-41a0-44ab-a6f0-1d84a381d610" />

5층 통로 모형에 층별 Hall Sensor를 배치하고 FS90R 연속회전 서보모터로 카를 구동한 프로토타입이다.

## Hardware

- MCU: STM32G474 (HAL)
- Drive: FS90R continuous-rotation servo, 50 Hz PWM
- Position sensing: Hall Sensor ×5
- Input: 4×4 Keypad
- Display: 7-Segment

## Software Structure

```text
Inc/
  elevator.h
  motor.h
  hall_sensor.h
  keypad.h
  display.h

Src/
  elevator.c      FSM / scheduling / safety state
  motor.c         PWM / open-loop ramp
  hall_sensor.c   floor detection / debounce
  keypad.c        keypad scan
  display.c       7-segment output
  main.c          HAL initialization / main loop
```

하드웨어 접근 코드와 운행 판단 로직을 분리했다. `elevator.c`는 GPIO나 PWM pulse 값을 직접 다루지 않고 `Motor_SetDir()`과 `Hall_DetectFloor()` 같은 의미 단위 인터페이스를 사용한다.

## Implementation

### FSM 기반 운행 제어

초기 구현은 이동 처리가 긴 `while` 루프를 점유해 운행, 도어 상태, 안전 처리가 하나의 제어 흐름에 결합되어 있었다.
이를 다음 5개 상태로 분리했다.

```text
IDLE
MOVING
DOOR_OPEN
DOOR_CLOSING
FAULT
```

`Elevator_Update()`는 호출될 때마다 현재 상태에 필요한 처리만 수행하고 반환한다. 메인 루프에서는 키패드 입력, FSM 갱신, 모터 ramp 갱신, 표시 출력을 반복 처리한다.

### SCAN 방식 호출 스케줄링

초기 구현은 `floor_requests[]`를 낮은 층부터 순차 탐색해 다음 목표 층을 선택했기 때문에 현재 이동 방향을 고려하지 못했다.

현재 구현은 진행 방향을 유지하면서 해당 방향에 있는 가장 가까운 요청을 먼저 선택한다. 진행 방향에 더 이상 요청이 없을 때만 방향을 반전한다.

```text
UP   : current floor보다 위쪽 요청 우선
DOWN : current floor보다 아래쪽 요청 우선

해당 방향 요청 없음
→ 방향 반전
→ 반대 방향 요청 처리
```

`pickTargetScan()`이 다음 목표 층을 결정하고, 이동 중 현재 지나가는 층에 요청이 있으면 해당 층에서 정지해 요청을 처리한다.

### Hall Sensor 기반 층 감지

각 층의 Hall Sensor를 이용해 현재 층을 판단한다.
센서가 한 번 활성화됐다는 이유만으로 바로 층으로 확정하지 않고, 약 `20 ms` 동안 같은 상태가 유지되는지 확인한 뒤 유효한 층 이벤트로 처리한다.

```text
Hall active
→ 20 ms 상태 확인
→ 유지됨: floor event
→ 풀림: noise로 무시
```

### Encoder 없는 구동계의 오픈루프 가감속

FS90R에는 속도 피드백용 Encoder가 없기 때문에 폐루프 PID 속도제어 대신 PWM pulse 기반 오픈루프 속도 프로파일을 사용했다.

- 출발: 목표 pulse까지 일정 step으로 ramp
- 순항: 정상 구동 pulse 유지
- 목표 층 한 층 전: Hall event를 기준으로 저속 pulse 전환
- 목표 층 감지: 정지 pulse로 전환
- 바로 옆 층이 목표인 경우: 처음부터 저속 구동

`Motor_Update()`는 `20 ms` 간격으로 현재 pulse를 목표 pulse 쪽으로 조금씩 이동시킨다.

### Safety Logic

- **Door state interlock**: `DOOR_OPEN` 상태에서는 모터 목표를 정지로 유지
- **Movement timeout**: 이동 중 `8 s` 안에 새로운 층 이벤트가 발생하지 않으면 `FAULT`로 전환하고 정지
- **FAULT state**: 모터 정지 상태를 유지해 추가 이동을 차단

이 프로젝트의 Door 상태는 운행 상태 머신에서 사용하는 논리 상태이며 별도의 Door actuator 제어는 포함하지 않는다.

## Why not PID?

현재 구동계는 FS90R의 실제 속도를 측정할 Encoder가 없다. 따라서 목표속도와 측정속도의 오차를 이용하는 폐루프 PID 제어 대신, 시간 기반 PWM ramp와 Hall Sensor 이벤트를 결합한 오픈루프 제어를 사용했다.

## Limitations

- 속도 피드백이 없어 부하나 전압 변화에 따른 실제 속도 편차를 폐루프로 보정할 수 없다.
- 짧은 층간 이동이나 연속 중간 정지에서는 충분한 가감속 거리를 확보하기 어렵다.
- Door는 소프트웨어 상태로만 모델링했으며 물리 Door actuator 제어는 구현 범위에 포함하지 않았다.

## Key Points

- 단일 제어 흐름을 FSM 기반 상태 처리 구조로 분리
- 이동 방향을 고려하지 않던 목표 선택을 SCAN 방식으로 개선
- Hall Sensor event를 이용한 층 감지 및 감속/정지
- Encoder가 없는 구동계에서 오픈루프 PWM ramp 적용
- Door state interlock과 이동 timeout 기반 FAULT 처리
