# 서리 창 스킬 세트

스킬 ID `1012`, 데이터 테이블 행 `FrostLance`.

## MCP 자동 검사

`Tools/Unreal/test_frost_lance_editor.py`의 내용을 네이티브 MCP
`editor_toolset.toolsets.programmatic.ProgrammaticToolset.execute_tool_script`의
`script` 인자로 전달하면 반복 실행할 수 있다. 도구가 제공하는 `execute_tool`을 사용하므로
일반 Python CLI에서 직접 실행하는 스크립트는 아니다.

현재 Unreal MCP 주소는 `http://127.0.0.1:8010/mcp`다.
처음 실행하는 MCP 세션에서는 `get_execution_environment`를 먼저 호출한다.
스크립트는 데이터를 수정하지 않으며 실패 항목과 전체 성공 여부를 JSON으로 반환한다.
도구 호출 자체가 실패하면 도구 오류로 보고된다.

실행 결과: 데이터·에셋 설정 검사 **36/36 통과**.
상세 결과는 `editor-smoke-result.json`에 저장했다.
이 검사는 실제 전투, 피해 적용, 시간 경과에 따른 빙결 해제, 수정 C++ 실행 검증을 포함하지 않는다.

## 게임 데이터

| 항목 | 설정 |
| --- | --- |
| 기본 피해 | 85 |
| 마나 | 12 |
| 쿨다운 | 2초 |
| 시전 지연 | 0.15초 |
| 투사체 속도 | 2,000 cm/s |
| 투사체 수명 | 1.5초 |
| 시전 애니메이션 | 기존 `AM_NormalAttack` 재사용 |

| Variant ID | 행 | 효과 |
| --- | --- | --- |
| 21 | `FrostLance_Pierce` | 본체가 최대 3명의 서로 다른 적을 타격 |
| 22 | `FrostLance_Fork` | 본체 적중 시 2개 분열체 생성, 분열체의 재분열 금지 |
| 23 | `FrostLance_Freeze` | 본체·분열체 적중 시 1.2초 이동 속도 0 및 Stunned·Freeze 태그 적용 |

빙결은 누적형이 아닌 즉시 적용형이다. 같은 적에게 재적용하면 지속시간을 갱신한다.
기존의 일반 FreezeStack variant(ID 13)는 수정하지 않았다.

## 프로젝트 연결

- `/Game/Datatable/DT_GASSkillData`: `FrostLance` 추가.
- `/Game/Datatable/DT_SkillVariantion`: 위 3개 variant 추가.
- `/Game/Datatable/DT_JobSkillSet`: `WarriorSkillSet.skillIds` 끝에 `1012` 추가.
- `GASData.CoolDown.1012`를 에디터 Gameplay Tags 도구로 등록.

새 스킬은 전사의 스킬 획득 보상 후보에 포함된다. 기존 시작 스킬과 퀵슬롯 순서는 유지한다.
현재 게임 인스턴스가 테이블을 이미 캐시했다면 PIE를 새로 시작해야 한다.
스킬 보유 상한은 기존 프로젝트 설정(8개)을 따른다.

## 에셋

모든 새 언리얼 에셋은 `/Game/Skills/FrostLance` 아래에 있다.

- `BP_FrostLance`: 기존 네이티브 `DiaProjectile`을 상속.
- `Meshes/SM_FrostLance`: Blender 제작, 120 triangles, 3 material slots, 원본 길이 160 cm. 액터에서 0.8 배율 사용.
- `Materials/M_FrostLance_Ice`, `M_FrostLance_Edge`, `M_FrostLance_Core`.
- `Effects/NS_FrostLance_Trail`, `NS_FrostLance_Impact`: 프로젝트의 `NS_IceSpike`를 기반으로 새로 구성. 신규 모델을 파편으로 사용.
- `GE_FrostLance_Freeze`: 프로젝트의 `GE_Stun2S`를 복제한 독립 효과. 이동 속도 Override 0, 고정 지속시간 1.2초.
- `Icons/T_FrostLance`, `T_FrostLance_Pierce`, `T_FrostLance_Fork`, `T_FrostLance_Freeze`: 512px Blender 렌더.

블렌더 원본·FBX·렌더는 프로젝트 루트의 `ArtSource/FrostLance`에 있다.
재생성 스크립트는 Source 기준 `Tools/Blender/create_frost_lance.py`다.
스크립트 재실행은 ArtSource 출력만 갱신하므로, 언리얼 에셋은 에디터에서 재임포트해야 한다.

## C++ 수정과 검증

기존 관통 처리에서는 variant마다 타격 횟수를 감소시키고 0이 된 횟수를 다시 초기화했다.
횟수 감소를 스킬 액터의 적중 처리로 옮겨 한 적중당 한 번만 감소하게 했다.
분열체도 정해진 타격 횟수 뒤에 제거하며, 장판 액터의 기존 예외는 유지한다.
Overlap 결과의 ImpactPoint가 비어 있으면 현재 액터 위치에서 적중 이펙트를 생성한다.

`ARPG.Skills.Variants.PreservesHitBudget` 자동화 테스트를 추가했다.
확인한 사항:

- 새 Blueprint와 머티리얼 컴파일 성공.
- 두 Niagara 시스템이 UpToDate, 오류 0, 경고 0.
- 스킬·variant의 기존 행을 편집 전 스냅샷과 대조하여 동일함을 확인.
- 직업 스킬 목록은 기존 10개 ID를 유지하고 1012만 추가.
- 모델을 언리얼에서 임포트하고 120 triangles, +X 축 방향, 머티리얼 연결 확인.
- 에셋과 데이터 테이블 저장 완료.

C++ 컴파일 및 수정 코드의 실제 플레이 검증은 아직 실행하지 않았다.
프로젝트 AGENTS.md가 명시적 빌드 요청을 요구하므로 사용자 선택을 기다리는 상태다.
현재 열린 에디터에는 기존 C++ 바이너리가 로드되어 있을 수 있다.

컴파일 이후 확인할 플레이 시나리오:

1. 보상 또는 프로젝트의 스킬 부여 경로로 1012 획득, 시전·마나·독립 쿨다운 확인.
2. variant 없이 한 적 타격 후 투사체 제거 확인.
3. 21 적용 후 일렬로 선 적 3명까지 타격 확인.
4. 22 적용 후 본체 적중마다 2개 분열체 및 재분열 금지 확인.
5. 23 적용 후 이동 속도 0·스킬 사용 제한, 1.2초 후 복구 확인.
6. 세 variant 동시 적용 시 관통 횟수 유지 및 분열체 빙결 확인.

`tables-before.json`은 편집 전 테이블 값이고 `verification.json`은 검증 결과다.
Native DataTableTools.set_rows는 전체 테이블을 재임포트하며 기존 InstancedStruct 안의 TMap 값을 잃을 수 있었다.
기존 SlowHit의 SetByCallerMagnitudes를 원래 Duration 2 / StatusValue 0.1로 복원한 뒤 모든 기존 행을 재대조했다.
