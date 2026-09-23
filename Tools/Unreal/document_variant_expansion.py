"""Generate the skill/variant catalogue from the verified native MCP snapshot."""
import json
from register_variant_expansion import OUT, load, canonical


def cell(value):
    return str(canonical(value)).replace('|', '\\|').replace('\n', ' ')


def route(row):
    if row['steps']:
        return ' → '.join(step['_structType'].split('.')[-1].removeprefix('GAS').removesuffix('StepData') for step in row['steps'])
    cls = row['abilityClass'].split('.')[-1].rstrip("'")
    return '기존 ' + cls if cls != 'DiaGameplayAbility' else 'Steps 없음(공통 Ability)'


def main():
    tables = load('tables-after.json')
    skills = sorted(tables['DT_GASSkillData'].values(), key=lambda r: r['skillId'])
    variants = sorted(tables['DT_SkillVariantion'].values(), key=lambda r: r['variantId'])
    by_id = {r['variantId']: r for r in variants}
    new_ids = {r['variantId'] for r in load('new-variants.json').values()}
    lines = [
        '# 스킬별 variant 할당', '',
        '2026-09-22 UnrealEditor 네이티브 MCP로 `DT_SkillVariantion`과 `DT_GASSkillData`를 수정하고 저장했습니다. 플레이어 스킬 15개에 variant 53개를 연결했습니다. 기존 32개를 모두 활용하며 21개를 새로 추가했습니다.', '',
        '이 목록은 각 스킬에서 획득·적용할 수 있는 variant 풀입니다. 플레이어에게 전부 지급하거나 자동 장착한 상태는 아닙니다. 기존 variant 보유·적용 구조를 그대로 사용합니다.', '',
        '테스트용 획득·적용 명령은 [스킬·variant 치트 안내](../SkillCheats.md)에 있습니다. `CheatGiveAllSkills` 다음 `CheatGiveAllSkillVariants`로 일괄 적용할 수 있습니다(C++ 빌드 필요).', '',
        '기존 `Steps`, 기본 피해·마나·쿨다운, 보스·몬스터 스킬 8개와 직업 스킬 풀은 보존했습니다. 새 동작은 공통 Ability의 Step 처리와 스킬 Actor/variant 처리에 추가했습니다. 기존 회피(1011)의 DiaDodgeAbility 경로는 유지하며 공통 시전 효과를 사용합니다.', '',
        '## 스킬별 구성', '',
        '별표(*)는 이번에 추가한 variant입니다. 서로 다른 효과를 함께 적용할 수 있습니다.', '',
        '| 스킬 | 실행 경로 | 허용 variant: ID 이름 |', '|---|---|---|',
    ]
    for row in skills:
        if 1001 <= row['skillId'] <= 1015:
            labels = [str(i) + ' ' + cell(by_id[i]['variantName']) + ('*' if i in new_ids else '') for i in row['variantIds']]
            lines.append(f"| {row['skillId']} {cell(row['skillName'])} | {route(row)} | {' / '.join(labels)} |")
    lines += ['', '## 조합 예시', '',
              '- 파이어볼트: 다중 발사(1) + 관통(2) + 연쇄(8) + 잔향 발사(49). 연속으로 발사하고 가까운 적으로 타격이 이어집니다.',
              '- 치유: 깊은 치유(36) + 회복 메아리(20) + 서리 성역(38) + 철갑 잔상(53). 회복 뒤 추가 회복과 방어 강화, 발밑 장판을 조합합니다.',
              '- 순간이동: 긴 점멸(42) + 서리 도착(43) + 가속 잔상(52). 이동 거리와 도착 지점 제어를 강화합니다.',
              '- 얼음 폭풍: 범위 확대(10) + 지속시간 증가(11) + 빙결 누적(13) + 소용돌이(19). 적을 장판 중심으로 당기며 반복 타격합니다.',
              '- 서리 돌진: 넓은 서릿발(30) + 지속시간 증가(11) + 소용돌이(19) + 서리 여진(51). 돌진은 한 번 실행하고 도착 장판만 추가 생성합니다.',
              '', '## 적용 규칙', '',
              '- 피해·범위·지속시간 계수는 곱으로 합성합니다. 다중 발사·관통·연쇄 수는 더합니다.',
              '- 연쇄는 본체 적중 후 450cm 이내 가까운 적 최대 2명에 기본 피해의 60%를 줍니다. 연쇄 타격에서는 추가 연쇄·분열·폭발을 호출하지 않습니다.',
              '- 흡혈은 실제로 줄어든 체력을 기준으로 계산하며 최대 체력을 넘지 않습니다.',
              '- 빙결 누적(13)은 3초 안에 3번 적중하면 1.2초 빙결을 적용하고 누적을 지웁니다. 별도 즉시 빙결 variant와 함께 쓰는 것도 허용합니다.',
              '- 회복 메아리는 0.6초 뒤 증가한 회복량의 50%를 회복합니다. 사망한 시전자는 회복하지 않습니다.',
              '- 시전 장판은 별도 기본 피해를 사용합니다. 치유량·순간이동 거리·강화 수치를 장판 피해로 사용하지 않습니다.',
              '- 가속·철갑 잔상은 같은 효과가 중첩되지 않습니다. 시전 효과는 적에게 부여되는 상태 효과와 구분합니다.',
              '- 기존 쿨다운·마나 variant의 설정값은 유지했습니다. 현재 코드는 이를 남는 비용/시간의 배수로 사용합니다(예: 0.2는 기존 시간의 20%).',
              '', '## 추가 variant 21개', '', '| ID | 이름 | 효과 |', '|---|---|---|']
    for row in variants:
        if row['variantId'] in new_ids:
            lines.append(f"| {row['variantId']} | {cell(row['variantName'])} | {cell(row['description'])} |")
    lines += ['', '## 에셋과 검증', '',
              '`/Game/Skills/Variants`에 GameplayEffect 4개(Haste, Ward, Mana, ChillStack)와 장판 Blueprint 2개(EmberField, RimeField)를 생성하고 Blueprint 컴파일·저장을 완료했습니다. 아이콘과 장판 외형은 기존 스킬 리소스를 재사용합니다.', '',
              '에디터 검증은 `editor-test-result.json`, 테이블 변경 검증은 `registration-result.json`에 있습니다. `tables-before.json`과 `tables-after.json`으로 변경 내용을 확인할 수 있습니다.', '',
              '확장 검사 175개, 기존 ElementalPack 검사 177개, FrostLance 검사 37개로 총 389개 에디터 검사가 통과했습니다. 검사 중 발견한 LavaEruption·ArcScatter·FrostRush의 Actor 기본 수명 5초는 기존 Step 값인 4초·1.2초·1.8초로 맞춰 컴파일·저장했습니다. 결과는 `verification-summary.json`에 있습니다.', '',
              '**C++ 빌드와 PIE 전투 테스트는 수행하지 않았습니다.** 프로젝트 AGENTS.md의 빌드 경계에 따라 소스 점검과 에디터 데이터·에셋 검사까지만 수행했습니다. 새 실행 동작은 C++ 빌드 후 검증해야 합니다. 기존 플레이 세션의 SkillObject는 허용 variant를 캐시하므로 빌드 후 새 PIE 세션에서 확인합니다.', '',
              '실행: `py -3 -X utf8 Tools/Unreal/test_variant_expansion_editor.py` (에디터 데이터 검사). 테이블 재등록은 `register_variant_expansion.py`를 사용하며 DataTable 쓰기는 각각 독립된 MCP 요청으로 처리합니다.', '']
    (OUT / 'README.md').write_text('\n'.join(lines), encoding='utf-8')

    catalogue = OUT.parent / 'Skills'
    (catalogue / 'catalog-snapshot.json').write_text(json.dumps(tables, ensure_ascii=False, indent=2), encoding='utf-8')
    lines = ['# 스킬 및 variant 목록', '',
             '2026-09-22 UnrealEditor 네이티브 MCP에서 읽은 저장 상태입니다. 스킬 23개(플레이어 15개, 보스·몬스터 8개), variant 53개가 등록돼 있습니다. 모든 variant는 하나 이상의 플레이어 스킬에 연결됐습니다.', '',
             '[이번 스킬별 구성과 조합 설명](../VariantExpansion/README.md). 아래 목록은 데이터 할당 결과이며 C++ 빌드·전투 검증은 아직 수행하지 않았습니다.', '',
             'Variant 획득·적용·해제 명령은 [치트 안내](../SkillCheats.md)를 참고하세요. `CheatGiveAllSkills` 다음 `CheatGiveAllSkillVariants`로 스킬별 할당을 모두 적용할 수 있습니다(C++ 빌드 필요).', '',
             '## 스킬 지급 치트', '', '```text', 'CheatGiveAllSkills', 'CheatGiveAllSkills true', 'CheatGiveSkill 1015 2', 'CheatListSkills', '```', '',
             '순서대로 플레이어 스킬 전체 지급, 보스·몬스터 포함 전체 지급, 서리 돌진을 2번 슬롯에 배치, 로드된 스킬과 허용 variant ID 조회입니다. C++ 빌드 후 사용할 수 있습니다. 전체 지급은 현재 플레이에서 보유 한도 8개를 우회하며 퀵슬롯은 5칸을 유지합니다. Variant 자동 획득·적용은 하지 않습니다.', '',
             '## 전체 스킬', '', '| ID | 이름 | 실행 경로 | 허용 variant ID |', '|---|---|---|---|']
    for row in skills:
        lines.append(f"| {row['skillId']} | {cell(row['skillName'])} | {route(row)} | {', '.join(map(str, row['variantIds'])) or '없음'} |")
    lines += ['', 'WarriorSkillSet에는 회피를 제외한 플레이어 스킬 14개가 등록돼 있습니다. 보스 패링/반격(2004)은 공통 Ability이지만 Steps가 비어 있어 별도 동작 확인이 필요합니다.', '',
              '## Variant', '', '| ID | 이름 | 설명 | 설정값 | 연결된 스킬 ID |', '|---|---|---|---|---|']
    for row in variants:
        linked = [str(s['skillId']) for s in skills if row['variantId'] in s['variantIds']]
        lines.append(f"| {row['variantId']} | {cell(row['variantName'])} | {cell(row['description'])} | {row['modifierValue']} | {', '.join(linked)} |")
    (catalogue / 'README.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(json.dumps({'skills': len(skills), 'variants': len(variants), 'new_variants': len(new_ids)}))


if __name__ == '__main__':
    main()
