# 일반 몬스터 3종

기존 메시와 애니메이션을 재사용한 근접형·원거리형·돌진형이다. 설정 원본은 `pack.json`, 등록 도구는 `Tools/Unreal/register_monster_pack_native.py`다. `.uasset` 파일은 직접 편집하지 않고 Unreal MCP로만 등록한다.

| 종류 | MonsterID | 체력 | 스킬 | 동작 |
|---|---|---:|---|---|
| 쌍날 보병 | `BladeGuard_01` | 260 | 3101 | 250cm 안에서 준비 동작 후 정면 베기 |
| 포자 투척병 | `Spitter_01` | 170 | 3102 | 900cm 안에서 투사체 발사, 추적 목표 거리 700cm |
| 황혼 돌격병 | `Charger_01` | 300 | 3101, 3103 | 가까우면 베기, 250~650cm에서는 준비 동작 후 500cm 돌진하고 도착 지점 공격 |

신규 스킬은 `UDiaGameplayAbility + Steps`로 구성한다. 돌진 Step 자체는 경로상의 피해를 적용하지 않으므로 도착 후 근접 판정 Step을 명시적으로 연결했다. 보병과 돌격병의 메시가 동일한 Skeleton을 사용하는 것을 확인했다. 투척병은 Bomber의 메시·AnimBP·몽타주 조합을 재사용한다.

돌진형은 전용 `AM_ChargerRush` 몽타주를 사용한다. 원본 `AM_DualBossDash`의 길이 0.833초보다 준비 시간 0.65초와 돌진 시간 0.45초의 합이 길어, 복제본의 `RateScale`을 0.5로 설정했다. 원본 몽타주는 변경하지 않았다. 일반 이동 속도는 기존 `InitializeMonsterAttributes`가 적용하는 600cm/s를 사용한다.

일반 전투방 `TestRoom`, `NewRow`의 기존 스폰 항목을 보존하면서 세 종류를 가중치 2·2·1로 추가한다. 보스 그룹과 방별 총 생성 수는 수정하지 않는다. 몬스터마다 일반 몬스터 태그와 쿨다운 태그를 사용하며 플레이어 스킬 보상 목록에는 추가하지 않는다.

## 소스 변경

- `ADiaMonster::InitializeFromData`: 기존 `FMonsterInfo.AttackRange` 값을 추적 거리로 보관한다.
- `BTTask_MoveToTarget`: 고정 150cm 대신 몬스터별 추적 거리를 사용한다. 목표 거리에 들어오면 추적을 멈추고, 캡슐 반경을 고려해 MoveTo 도달 조건과 AI 거리 조건을 맞춘다.
- `BTTask_RotateToTarget`: 공격 준비 중 기존 추적 요청이 계속 이동시키지 않도록 멈춘다.

## 등록과 복구

1. 에디터를 열고 `127.0.0.1:8010`의 리스너가 UnrealEditor인지 확인한다.
2. PIE를 종료한 상태에서 외부 Python으로 `Tools/Unreal/register_monster_pack_native.py`를 실행한다.
3. 도구는 현재 테이블을 JSON으로 보관하고, 새 Blueprint·AI 트리를 구성·검증·저장한다.
4. 데이터 테이블은 개별 MCP 요청으로 순차 갱신한다. 기존 행 보존과 추가 데이터 재조회가 통과한 테이블만 저장한다.
5. 최종 결과는 `registration-result.json`과 `tables-after.json`에 기록한다.

**ProgrammaticToolset 안에서 DataTable add_rows/set_rows를 실행하면 안 된다.** 전체 Undo 트랜잭션과 DataTable 객체 재생성이 충돌하는 기존 장애가 `docs/ElementalPack/crash-diagnosis.md`에 기록돼 있다. 이번 작업에서도 이 경로에서 에디터가 종료되었으며, 에디터 재실행 후 외부 클라이언트의 개별 요청 방식으로 등록·재조회·저장을 완료했다.

Native DataTableTools는 `Name`을 행 키로 예약한다. 몬스터 표시 이름은 설정 파일에 보존하지만 이 도구로 `FMonsterInfo.Name`을 쓰지 않는다. 현재 UI에는 몬스터 이름 표시가 없으며, 행 ID로 등록한다.

## 검증 상태

- ARPGEditor Win64 Development 빌드 통과.
- Python 등록 스크립트 구문 검사 통과.
- Blueprint 3종을 경고도 오류로 취급하는 옵션으로 컴파일하고 속성을 다시 읽어 검증했다.
- 새 에셋 7개와 데이터 테이블 3개 저장 완료. `final-save-check.json`에서 모두 `dirty: false`이며 PIE는 종료했다.
- 테이블 갱신 전후 비교에서 기존 행과 스폰 항목이 보존됐다. 쿨다운 태그 3101·3102·3103도 등록했다.
- TestRoom의 일반 전투방에 입장해 세 종류가 모두 생성되고, 일반 AI 컨트롤러가 플레이어를 추적하며 스킬을 사용하는 것을 확인했다. 생성 기록은 `pie-entered4.json`에 있다.
- 공격별 확인에서는 다른 몬스터의 AI를 일시 정지하고 위치를 고정한 뒤 실제 스킬을 발동했다. 세 종류 모두 애니메이션 재생, 플레이어 체력 감소, 스킬 종료를 확인했다. 돌진형의 이동량은 약 500cm였다. 결과는 `pie-*-attack.json`에 있다.
- 원거리형은 1200cm에서 AI를 재개한 뒤 접근하고 반복 공격했다. 에디터 백그라운드의 낮은 프레임률에서는 목표 거리 700cm를 지나 약 473cm에서 멈췄다. 설정값은 목표 접근 거리이며 일정 간격을 유지하며 후퇴하는 AI는 아니다. 기록은 `pie-spitter-ai.json`에 있다.
- PIE 검증은 자동 점검이다. 조작감, 외형, 다양한 지형에서의 이동은 수동 플레이로 추가 확인할 수 있다.
- 밸런스 수치는 초기값이며 실제 플레이 조정이 필요하다.
