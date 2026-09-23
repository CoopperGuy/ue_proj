# 스킬별 variant 할당

2026-09-22 UnrealEditor 네이티브 MCP로 `DT_SkillVariantion`과 `DT_GASSkillData`를 수정하고 저장했습니다. 플레이어 스킬 15개에 variant 53개를 연결했습니다. 기존 32개를 모두 활용하며 21개를 새로 추가했습니다.

이 목록은 각 스킬에서 획득·적용할 수 있는 variant 풀입니다. 플레이어에게 전부 지급하거나 자동 장착한 상태는 아닙니다. 기존 variant 보유·적용 구조를 그대로 사용합니다.

테스트용 획득·적용 명령은 [스킬·variant 치트 안내](../SkillCheats.md)에 있습니다. `CheatGiveAllSkills` 다음 `CheatGiveAllSkillVariants`로 일괄 적용할 수 있습니다(C++ 빌드 필요).

기존 `Steps`, 기본 피해·마나·쿨다운, 보스·몬스터 스킬 8개와 직업 스킬 풀은 보존했습니다. 새 동작은 공통 Ability의 Step 처리와 스킬 Actor/variant 처리에 추가했습니다. 기존 회피(1011)의 DiaDodgeAbility 경로는 유지하며 공통 시전 효과를 사용합니다.

## 스킬별 구성

별표(*)는 이번에 추가한 variant입니다. 서로 다른 효과를 함께 적용할 수 있습니다.

| 스킬 | 실행 경로 | 허용 variant: ID 이름 |
|---|---|---|
| 1001 기본 공격 | ProjectileSpawn | 1 Multiple Shot / 2 Pierce / 16 Knockback / 14 Burning Ground / 3 Damage Up / 6 Slow Hit / 7 Explode On Hit / 9 Fork / 15 Life Steal / 17 Bigger But Slower / 18 Shotgun |
| 1002 강력한 공격 | MeleeSpawn | 4 Cooldown Cut / 5 Mana Efficient / 33 분쇄 타격* / 34 넓은 가르기* / 35 얼어붙은 칼날* |
| 1003 파이어볼트 | ProjectileSpawn | 1 Multiple Shot / 2 Pierce / 3 Damage Up / 5 Mana Efficient / 7 Explode On Hit / 9 Fork / 14 Burning Ground / 6 Slow Hit / 8 Chain / 15 Life Steal / 17 Bigger But Slower / 49 잔향 발사* |
| 1004 치유 | Utility | 4 Cooldown Cut / 5 Mana Efficient / 36 깊은 치유* / 20 Echo / 38 서리 성역* / 53 철갑 잔상* / 54 마력 환류* |
| 1005 질주 | Utility | 4 Cooldown Cut / 5 Mana Efficient / 39 바람 가속* / 40 끝없는 질주* / 41 불꽃 출발* / 52 가속 잔상* |
| 1006 다중 사격 | ProjectileSpawn | 2 Pierce / 3 Damage Up / 4 Cooldown Cut / 5 Mana Efficient / 7 Explode On Hit / 9 Fork / 1 Multiple Shot / 6 Slow Hit / 15 Life Steal / 17 Bigger But Slower / 18 Shotgun |
| 1007 대지 창 | GroundSpawn | 3 Damage Up / 4 Cooldown Cut / 5 Mana Efficient / 10 Area Up / 12 Repeat Cast / 6 Slow Hit / 16 Knockback |
| 1008 얼음 폭풍 | GroundSpawn | 3 Damage Up / 4 Cooldown Cut / 5 Mana Efficient / 10 Area Up / 11 Duration Up / 6 Slow Hit / 12 Repeat Cast / 13 Freeze Build Up / 15 Life Steal / 19 Vortex / 50 절대영도* |
| 1009 순간이동 | Utility | 4 Cooldown Cut / 5 Mana Efficient / 42 긴 점멸* / 43 서리 도착* / 52 가속 잔상* / 54 마력 환류* |
| 1010 광전사 | Utility | 4 Cooldown Cut / 5 Mana Efficient / 44 광폭한 힘* / 45 끝나지 않는 분노* / 46 불타는 투기장* / 53 철갑 잔상* / 54 마력 환류* |
| 1011 회피 | 기존 DiaDodgeAbility | 47 가벼운 몸놀림* / 52 가속 잔상* / 53 철갑 잔상* |
| 1012 서리 창 | ProjectileSpawn | 21 꿰뚫는 서리 / 22 갈라지는 서리 / 23 순간 빙결 / 1 Multiple Shot / 3 Damage Up / 6 Slow Hit / 13 Freeze Build Up / 15 Life Steal / 17 Bigger But Slower / 48 얼음 파쇄* / 49 잔향 발사* |
| 1013 용암 분출 | GroundSpawn | 24 넘치는 용암 / 25 재분출 / 26 안정된 화맥 / 3 Damage Up / 6 Slow Hit / 11 Duration Up / 12 Repeat Cast / 15 Life Steal / 19 Vortex |
| 1014 전격 산탄 | ProjectileSpawn | 27 다중 방전 / 28 관통 전류 / 29 가지 번개 / 3 Damage Up / 6 Slow Hit / 7 Explode On Hit / 8 Chain / 13 Freeze Build Up / 15 Life Steal / 17 Bigger But Slower / 18 Shotgun / 49 잔향 발사* |
| 1015 서리 돌진 | Charge → GroundSpawn | 30 넓은 서릿발 / 31 동결 지대 / 32 겨울 걸음 / 3 Damage Up / 6 Slow Hit / 11 Duration Up / 13 Freeze Build Up / 15 Life Steal / 19 Vortex / 51 서리 여진* / 53 철갑 잔상* |

## 조합 예시

- 파이어볼트: 다중 발사(1) + 관통(2) + 연쇄(8) + 잔향 발사(49). 연속으로 발사하고 가까운 적으로 타격이 이어집니다.
- 치유: 깊은 치유(36) + 회복 메아리(20) + 서리 성역(38) + 철갑 잔상(53). 회복 뒤 추가 회복과 방어 강화, 발밑 장판을 조합합니다.
- 순간이동: 긴 점멸(42) + 서리 도착(43) + 가속 잔상(52). 이동 거리와 도착 지점 제어를 강화합니다.
- 얼음 폭풍: 범위 확대(10) + 지속시간 증가(11) + 빙결 누적(13) + 소용돌이(19). 적을 장판 중심으로 당기며 반복 타격합니다.
- 서리 돌진: 넓은 서릿발(30) + 지속시간 증가(11) + 소용돌이(19) + 서리 여진(51). 돌진은 한 번 실행하고 도착 장판만 추가 생성합니다.

## 적용 규칙

- 피해·범위·지속시간 계수는 곱으로 합성합니다. 다중 발사·관통·연쇄 수는 더합니다.
- 연쇄는 본체 적중 후 450cm 이내 가까운 적 최대 2명에 기본 피해의 60%를 줍니다. 연쇄 타격에서는 추가 연쇄·분열·폭발을 호출하지 않습니다.
- 흡혈은 실제로 줄어든 체력을 기준으로 계산하며 최대 체력을 넘지 않습니다.
- 빙결 누적(13)은 3초 안에 3번 적중하면 1.2초 빙결을 적용하고 누적을 지웁니다. 별도 즉시 빙결 variant와 함께 쓰는 것도 허용합니다.
- 회복 메아리는 0.6초 뒤 증가한 회복량의 50%를 회복합니다. 사망한 시전자는 회복하지 않습니다.
- 시전 장판은 별도 기본 피해를 사용합니다. 치유량·순간이동 거리·강화 수치를 장판 피해로 사용하지 않습니다.
- 가속·철갑 잔상은 같은 효과가 중첩되지 않습니다. 시전 효과는 적에게 부여되는 상태 효과와 구분합니다.
- 기존 쿨다운·마나 variant의 설정값은 유지했습니다. 현재 코드는 이를 남는 비용/시간의 배수로 사용합니다(예: 0.2는 기존 시간의 20%).

## 추가 variant 21개

| ID | 이름 | 효과 |
|---|---|---|
| 33 | 분쇄 타격 | 강력한 공격의 기본 피해가 50% 증가합니다. |
| 34 | 넓은 가르기 | 강력한 공격의 판정 반경과 각도가 50% 증가합니다. 각도는 최대 360도입니다. |
| 35 | 얼어붙은 칼날 | 강력한 공격에 맞은 적에게 2초 동안 50% 둔화를 적용합니다. |
| 36 | 깊은 치유 | 치유 회복량이 60% 증가합니다. 회복 메아리에도 증가한 회복량을 사용합니다. |
| 38 | 서리 성역 | 치유 시 발밑에 반경 200cm의 서리 장판을 2초간 남깁니다. 0.5초마다 기본 피해 18과 2초 둔화를 줍니다. |
| 39 | 바람 가속 | 질주의 추가 이동 속도가 40% 증가합니다. |
| 40 | 끝없는 질주 | 질주의 속도 강화 지속시간이 60% 증가합니다. |
| 41 | 불꽃 출발 | 질주를 시작한 지점에 반경 160cm의 용암 장판을 2초간 남깁니다. 0.5초마다 기본 피해 18을 줍니다. |
| 42 | 긴 점멸 | 순간이동 최대 거리가 50% 증가합니다. |
| 43 | 서리 도착 | 순간이동 후 현재 위치에 반경 240cm의 서리 장판을 2초간 생성합니다. 0.5초마다 기본 피해 20과 2초 둔화를 줍니다. |
| 44 | 광폭한 힘 | 광전사의 추가 공격력이 50% 증가합니다. 기존 방어력 감소는 유지합니다. |
| 45 | 끝나지 않는 분노 | 광전사 강화와 방어력 감소의 지속시간이 50% 증가합니다. |
| 46 | 불타는 투기장 | 광전사 시전 지점에 반경 200cm의 용암 장판을 3초간 생성합니다. 0.5초마다 기본 피해 20을 줍니다. |
| 47 | 가벼운 몸놀림 | 회피의 재사용 대기시간이 30% 감소합니다. |
| 48 | 얼음 파쇄 | 서리 창 본체가 적중하면 반경 180cm에 추가 폭발 피해를 줍니다. |
| 49 | 잔향 발사 | 0.35초 후 발사 단계를 한 번 더 실행합니다. 다중 발사와 함께 사용할 수 있습니다. |
| 50 | 절대영도 | 얼음 폭풍에 적중한 적에게 1.2초 빙결을 적용합니다. |
| 51 | 서리 여진 | 돌진 도착 지점의 장판 생성 단계를 0.4초 뒤 한 번 더 실행합니다. 돌진은 반복하지 않습니다. |
| 52 | 가속 잔상 | 스킬 시전 시 자신에게 2초 동안 이동 속도 +120을 부여합니다. 같은 효과는 중첩되지 않습니다. |
| 53 | 철갑 잔상 | 스킬 시전 시 자신에게 3초 동안 방어력 +20을 부여합니다. 같은 효과는 중첩되지 않습니다. |
| 54 | 마력 환류 | 스킬 시전 시 자신에게 마나 12를 돌려줍니다. |

## 에셋과 검증

`/Game/Skills/Variants`에 GameplayEffect 4개(Haste, Ward, Mana, ChillStack)와 장판 Blueprint 2개(EmberField, RimeField)를 생성하고 Blueprint 컴파일·저장을 완료했습니다. 아이콘과 장판 외형은 기존 스킬 리소스를 재사용합니다.

에디터 검증은 `editor-test-result.json`, 테이블 변경 검증은 `registration-result.json`에 있습니다. `tables-before.json`과 `tables-after.json`으로 변경 내용을 확인할 수 있습니다.

확장 검사 175개, 기존 ElementalPack 검사 177개, FrostLance 검사 37개로 총 389개 에디터 검사가 통과했습니다. 검사 중 발견한 LavaEruption·ArcScatter·FrostRush의 Actor 기본 수명 5초는 기존 Step 값인 4초·1.2초·1.8초로 맞춰 컴파일·저장했습니다. 결과는 `verification-summary.json`에 있습니다.

**C++ 빌드와 PIE 전투 테스트는 수행하지 않았습니다.** 프로젝트 AGENTS.md의 빌드 경계에 따라 소스 점검과 에디터 데이터·에셋 검사까지만 수행했습니다. 새 실행 동작은 C++ 빌드 후 검증해야 합니다. 기존 플레이 세션의 SkillObject는 허용 variant를 캐시하므로 빌드 후 새 PIE 세션에서 확인합니다.

실행: `py -3 -X utf8 Tools/Unreal/test_variant_expansion_editor.py` (에디터 데이터 검사). 테이블 재등록은 `register_variant_expansion.py`를 사용하며 DataTable 쓰기는 각각 독립된 MCP 요청으로 처리합니다.
