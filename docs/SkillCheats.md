# 스킬·variant 치트

C++ 반영 후 PIE에서 캐릭터를 조종하고 게임 화면을 클릭한 뒤 `~` 키로 콘솔을 엽니다. 프로젝트 콘솔 키는 `Tilde`로 설정돼 있습니다. 이 명령은 게임 콘솔에서 실행합니다.

```text
CheatListSkills
CheatGiveAllSkills
CheatGiveSkill 1012
CheatGiveSkill 1013
CheatGiveSkill 1014
CheatGiveSkill 1015
```

1012는 서리 창, 1013은 용암 분출, 1014는 전격 산탄, 1015는 서리 돌진입니다. 스킬을 지급하고 빈 퀵슬롯에 자동 배치합니다. 이미 보유한 스킬은 중복 지급하거나 레벨을 올리지 않습니다.

`CheatGiveAllSkills`는 로드된 테이블에서 `SkillType.Character.Player` 태그가 있는 모든 스킬을 지급합니다. 현재 1001~1015의 15개가 대상입니다. `CheatGiveAllSkills true`는 보스·몬스터용까지 포함한 전체 23개를 대상으로 합니다. 지급 성공·기존 보유·실패 수를 출력하며, 테이블 등록 자체가 전투 동작을 보장하지는 않습니다.

전체 지급 호출에서만 보유 한도 8개를 우회합니다. 일반 보상과 단일 지급 치트의 한도는 유지되며, 퀵슬롯은 5칸입니다. 기존 퀵슬롯을 유지하고 새 스킬은 남은 빈 칸에 자동 배치합니다. 초과분은 보유 목록에서 선택하거나 `CheatGiveSkill 1015 2`처럼 지정 슬롯에 배치합니다. 전체 지급을 반복해도 기존 스킬을 중복 부여하거나 레벨을 올리지 않습니다.

Variant는 전체 스킬 지급에 포함되지 않으며 자동 적용하지 않습니다. `CheatListSkills`는 스킬별 허용 variant ID도 출력합니다. 등록된 스킬/variant 전체 목록과 연결 관계는 [스킬 카탈로그](Skills/README.md)에 있습니다.

## Variant 획득·적용

2026-09-23 추가한 명령입니다. C++ 빌드 후 새 PIE 세션의 게임 콘솔에서 다음 순서로 실행하면 플레이어 스킬과 스킬별로 할당된 variant를 모두 획득·적용합니다.

```text
CheatGiveAllSkills
CheatGiveAllSkillVariants
```

| 명령 | 동작 |
|---|---|
| `CheatGiveSkillVariant 1014 8` | 전격 산탄에 할당된 연쇄(8)를 획득하고 적용 |
| `CheatGiveSkillVariant 1014 8 false` | 연쇄를 획득. 현재 적용 상태는 유지 |
| `CheatGiveAllSkillVariants 1014` | 전격 산탄에 할당된 모든 variant 획득·적용 |
| `CheatGiveAllSkillVariants 0 false` | 모든 보유 스킬의 할당 variant 획득. 현재 적용 상태는 유지 |
| `CheatClearSkillVariants 1014` | 전격 산탄의 variant 적용을 모두 해제. 획득 상태는 유지 |
| `CheatClearSkillVariants` | 모든 보유 스킬의 variant 적용 해제 |
| `CheatListSkillVariants 1014` | 전격 산탄의 variant ID·이름·설명·보유/적용 상태 조회 |
| `CheatListSkillVariants` | 모든 보유 스킬의 variant 목록 조회 |

단일 지급은 `스킬ID variantID [bApply=true]`, 일괄 지급은 `[스킬ID=0] [bApply=true]` 순서입니다. 일괄 지급·해제·조회에서 스킬 ID 생략 또는 0은 **현재 보유한 모든 스킬**을 뜻합니다. 미보유 스킬은 먼저 `CheatGiveSkill` 또는 `CheatGiveAllSkills`로 지급해야 합니다. 특정 스킬 ID를 지정한 목록 조회는 미보유 상태에서도 가능합니다.

스킬 테이블의 `VariantIDs`와 현재 SkillObject의 허용 목록을 모두 확인합니다. 다른 스킬 전용 variant, 잘못된 ID, 누락된 데이터는 거절하고 이유를 출력합니다. 반복 실행해도 중복 획득·적용되지 않습니다. 결과의 할당 건수는 **스킬과 variant의 조합 수**이므로 공통 variant가 여러 스킬에 할당되면 각각 집계합니다.

예를 들어 전격 산탄에서 다중 방전·연쇄·잔향 발사 조합만 시험하려면 다음과 같이 실행합니다.

```text
CheatGiveAllSkills
CheatGiveSkill 1014 1
CheatClearSkillVariants 1014
CheatGiveSkillVariant 1014 27
CheatGiveSkillVariant 1014 8
CheatGiveSkillVariant 1014 49
CheatListSkillVariants 1014
```

기존 `TryUnlockSkillVariantByID`와 `TryApplySkillVariantByID`를 사용하므로 Step 실행에서도 같은 적용 목록을 읽습니다. 해제 후 다음 시전으로 비교하세요. 해제 명령은 현재 투사체·장판을 삭제하거나 이미 걸린 일시 강화를 제거하지 않습니다. 단독 플레이/서버 플레이어에서 변경할 수 있으며 Shipping에서는 실행되지 않습니다. 획득·적용 상태는 기존 저장 시스템의 저장 대상입니다.

## 퀵슬롯과 스킬 지급 제한

슬롯을 직접 지정하려면 `CheatGiveSkill 1013 2`처럼 입력합니다. 슬롯 번호는 1~5이며 기존 슬롯의 스킬은 보유 목록에 남습니다. 지정한 스킬이 다른 슬롯에 있었다면 이전 슬롯을 비웁니다. 슬롯을 생략하거나 0으로 지정하면 기존 배치를 유지하거나 빈 슬롯에 넣습니다.

빈 퀵슬롯이 없어도 스킬은 보유 목록에 추가됩니다. 이후 슬롯 번호를 지정해 다시 실행하면 사용할 수 있습니다. 보유 한도 8개를 초과하면 지급을 거절하고 이유를 출력합니다. 잘못된 ID·슬롯, 캐릭터 부재, 지급 실패도 게임 콘솔과 Output Log에 표시합니다.

기존 `ApplySkillByID` 및 캐릭터의 퀵슬롯 경로를 사용하므로 GAS 능력과 HUD를 함께 갱신합니다. 단독 플레이/서버 플레이어용이며 Shipping 빌드에서는 실행되지 않습니다. 보유 스킬은 기존 저장 시스템의 저장 대상이 될 수 있습니다.

전체 지급은 현재 플레이 테스트용입니다. 저장 후 불러오기에는 일반 보유 한도 8개가 적용되므로, 초과 스킬까지 영구 복원하는 기능은 포함하지 않습니다. 다시 테스트할 때는 전체 지급 명령을 재실행합니다.

## 검증 상태

선언·정의, 호출 대상, 중복 지급 처리, 입력값 검사, 기존 등록 경로와 HUD 갱신 경로를 소스로 확인했습니다. Variant 치트는 할당 검사, 기존 획득·적용 함수 연결, 적용 해제 시 보유 목록 보존, 서버/Shipping 조건을 소스로 확인했습니다. 프로젝트 AGENTS.md에 따라 Unreal 빌드는 실행하지 않았으며 컴파일 및 PIE 동작은 아직 검증하지 않았습니다. 새 UFUNCTION이 포함되어 있으므로 사용 전에 에디터 타깃 C++ 빌드가 필요합니다.

빌드 후 확인할 항목:

- `CheatListSkills`에 1012~1015가 나타나는지 확인합니다.
- `CheatGiveSkill 1013` 후 퀵슬롯 표시와 스킬 사용을 확인합니다.
- 같은 명령을 반복해 보유 스킬 수와 능력이 중복 증가하지 않는지 확인합니다.
- `CheatGiveSkill 1013 2`로 이동 후 이전 슬롯 해제와 2번 슬롯 사용을 확인합니다.
- 존재하지 않는 ID, 슬롯 6, 보유 한도 및 퀵슬롯 한도에서 결과 안내를 확인합니다.
- `CheatGiveAllSkills` 실행 후 플레이어 스킬 15개를 보유하고, 반복 실행 시 증가하지 않는지 확인합니다.
- `CheatGiveAllSkills true` 실행 시 보스·몬스터용 8개까지 추가되는지 확인합니다.
- 전체 지급 후 기존 퀵슬롯과 적용 variant가 유지되고, 슬롯을 지정하면 초과 보유 스킬도 사용할 수 있는지 확인합니다.
- `CheatGiveAllSkillVariants`를 두 번 실행해 두 번째 신규 획득·적용 수가 0이고 실패가 없는지 확인합니다.
- `CheatClearSkillVariants 1014` 후 목록이 보유·미적용이고 다른 스킬의 적용 상태는 유지되는지 확인합니다.
- `CheatGiveSkillVariant 1014 8 false` 후 미적용 상태, 같은 명령에서 `true` 사용 후 적용 상태를 확인합니다.
- `CheatGiveSkillVariant 1014 36`으로 다른 스킬 전용 variant가 거절되는지 확인합니다.
- 위 전격 산탄 조합으로 실제 다중 발사·연쇄·추가 발사를 확인합니다. 일괄 적용과 개별 조합을 모두 비교합니다.
