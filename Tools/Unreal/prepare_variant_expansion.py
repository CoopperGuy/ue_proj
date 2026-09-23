"""Build reviewed variant rows and per-skill allowlists from the MCP snapshot."""
import copy
import json
from pathlib import Path

OUT = Path(__file__).resolve().parents[2] / 'docs/VariantExpansion'


def main():
    before = json.loads((OUT / 'tables-before.json').read_text(encoding='utf-8'))
    skills = before['DT_GASSkillData']
    variants = before['DT_SkillVariantion']
    by_id = {r['skillId']: r for r in skills.values()}
    rows = {}

    def effect(path):
        return {'_structType': '/Script/ARPG.SkillVariantGameplayEffectData',
                'TargetEffects': [{'EffectClass': "/Script/Engine.BlueprintGeneratedClass'" + path + "_C'", 'SetByCallerMagnitudes': {}}]}

    def field(name, radius, seconds, damage, slow=False):
        data = {'_structType': '/Script/ARPG.SkillVariantGroundSpawnData',
                'SkillActorClass': "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/Variants/BP_Variant_" + name + '.BP_Variant_' + name + "_C'",
                'Radius': radius, 'Duration': seconds, 'TickInterval': .5, 'DamageEffectClass': 'None', 'TargetEffects': []}
        if slow:
            data['TargetEffects'] = effect('/Game/Skills/FrostRush/GE_FrostRush_Slow.GE_FrostRush_Slow')['TargetEffects']
        return data

    def add(key, vid, title, description, tag, value, skill, extra=None):
        row = copy.deepcopy(variants['DamageUp'])
        row.update(variantId=vid, variantName=title, description=description, modifierValue=value,
                   variantTag={'TagName': 'GASData.Variant.' + tag}, icon=by_id[skill]['icon'], variantExtraData=extra or {})
        rows[key] = row

    add('PowerStrike_Heavy', 33, '분쇄 타격', '강력한 공격의 기본 피해가 50% 증가합니다.', 'DamageMultiplier', 1.5, 1002)
    add('PowerStrike_Sweep', 34, '넓은 가르기', '강력한 공격의 판정 반경과 각도가 50% 증가합니다. 각도는 최대 360도입니다.', 'AreaMultiplier', 1.5, 1002)
    add('PowerStrike_Chill', 35, '얼어붙은 칼날', '강력한 공격에 맞은 적에게 2초 동안 50% 둔화를 적용합니다.', 'SlowOnHit', .5, 1002,
        effect('/Game/Skills/FrostRush/GE_FrostRush_Slow.GE_FrostRush_Slow'))
    add('Heal_Deep', 36, '깊은 치유', '치유 회복량이 60% 증가합니다. 회복 메아리에도 증가한 회복량을 사용합니다.', 'DamageMultiplier', 1.6, 1004)
    add('Heal_Sanctuary', 38, '서리 성역', '치유 시 발밑에 반경 200cm의 서리 장판을 2초간 남깁니다. 0.5초마다 기본 피해 18과 2초 둔화를 줍니다.',
        'CastGround', 18, 1004, field('RimeField', 200, 2, 18, True))
    add('Sprint_Surge', 39, '바람 가속', '질주의 추가 이동 속도가 40% 증가합니다.', 'DamageMultiplier', 1.4, 1005)
    add('Sprint_Endurance', 40, '끝없는 질주', '질주의 속도 강화 지속시간이 60% 증가합니다.', 'DurationMultiplier', 1.6, 1005)
    add('Sprint_ScorchedStart', 41, '불꽃 출발', '질주를 시작한 지점에 반경 160cm의 용암 장판을 2초간 남깁니다. 0.5초마다 기본 피해 18을 줍니다.',
        'CastGround', 18, 1005, field('EmberField', 160, 2, 18))
    add('Teleport_Long', 42, '긴 점멸', '순간이동 최대 거리가 50% 증가합니다.', 'DamageMultiplier', 1.5, 1009)
    add('Teleport_Rime', 43, '서리 도착', '순간이동 후 현재 위치에 반경 240cm의 서리 장판을 2초간 생성합니다. 0.5초마다 기본 피해 20과 2초 둔화를 줍니다.',
        'CastGround', 20, 1009, field('RimeField', 240, 2, 20, True))
    add('Berserker_Fury', 44, '광폭한 힘', '광전사의 추가 공격력이 50% 증가합니다. 기존 방어력 감소는 유지합니다.', 'DamageMultiplier', 1.5, 1010)
    add('Berserker_Endurance', 45, '끝나지 않는 분노', '광전사 강화와 방어력 감소의 지속시간이 50% 증가합니다.', 'DurationMultiplier', 1.5, 1010)
    add('Berserker_Battlefield', 46, '불타는 투기장', '광전사 시전 지점에 반경 200cm의 용암 장판을 3초간 생성합니다. 0.5초마다 기본 피해 20을 줍니다.',
        'CastGround', 20, 1010, field('EmberField', 200, 3, 20))
    add('Dodge_Quick', 47, '가벼운 몸놀림', '회피의 재사용 대기시간이 30% 감소합니다.', 'CooldownReduction', .7, 1011)
    add('FrostLance_Shatter', 48, '얼음 파쇄', '서리 창 본체가 적중하면 반경 180cm에 추가 폭발 피해를 줍니다.', 'ExplodeOnHit', 180, 1012)
    add('Projectile_EchoCast', 49, '잔향 발사', '0.35초 후 발사 단계를 한 번 더 실행합니다. 다중 발사와 함께 사용할 수 있습니다.', 'RepeatCast', .35, 1003)
    add('IceRain_AbsoluteZero', 50, '절대영도', '얼음 폭풍에 적중한 적에게 1.2초 빙결을 적용합니다.', 'FreezeStack', 1.2, 1008,
        effect('/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze'))
    add('FrostRush_Aftershock', 51, '서리 여진', '돌진 도착 지점의 장판 생성 단계를 0.4초 뒤 한 번 더 실행합니다. 돌진은 반복하지 않습니다.', 'RepeatCast', .4, 1015)
    for key, vid, title, desc, ge, sid in [
        ('Cast_Haste', 52, '가속 잔상', '스킬 시전 시 자신에게 2초 동안 이동 속도 +120을 부여합니다. 같은 효과는 중첩되지 않습니다.', 'Haste', 1011),
        ('Cast_Ward', 53, '철갑 잔상', '스킬 시전 시 자신에게 3초 동안 방어력 +20을 부여합니다. 같은 효과는 중첩되지 않습니다.', 'Ward', 1011),
        ('Cast_Mana', 54, '마력 환류', '스킬 시전 시 자신에게 마나 12를 돌려줍니다.', 'Mana', 1004)]:
        add(key, vid, title, desc, 'CastSelfEffect', 1, sid, effect('/Game/Skills/Variants/GE_Variant_' + ge + '.GE_Variant_' + ge))

    updates = {
        'Chain': {'description': '본체 적중 후 450cm 이내 가까운 적을 최대 2명 연쇄 타격합니다. 추가 타격의 기본 피해는 60%이며 재연쇄·분열·폭발은 발생하지 않습니다.'},
        'FreezeStack': {'description': '3초 이내 연속 3회 적중하면 1.2초 빙결을 적용하고 누적을 초기화합니다.',
                        'variantExtraData': effect('/Game/Skills/Variants/GE_Variant_ChillStack.GE_Variant_ChillStack')},
        'BigProjectile': {'description': '투사체 크기 60%, 기본 피해 35% 증가. 투사체 속도는 30% 감소합니다.'},
        'CloseRangeBonus': {'description': '시전자와 대상의 거리가 350cm 이하일 때 기본 피해가 50% 증가합니다.'},
        'Vortex': {'description': '장판이 타격할 때 적을 중심 방향으로 끌어당깁니다. 중심 50cm 안에서는 당기지 않습니다.'},
        'Echo': {'description': '치유 사용 0.6초 뒤 회복량의 50%를 추가 회복합니다. 사망한 시전자를 되살리지는 않습니다.'},
    }
    additions = {
        1001: [3, 6, 7, 9, 15, 17, 18], 1002: [33, 34, 35],
        1003: [6, 8, 15, 17, 49], 1004: [36, 20, 38, 53, 54],
        1005: [39, 40, 41, 52], 1006: [1, 6, 15, 17, 18],
        1007: [6, 16], 1008: [6, 12, 13, 15, 19, 50],
        1009: [42, 43, 52, 54], 1010: [44, 45, 46, 53, 54],
        1011: [47, 52, 53], 1012: [1, 3, 6, 13, 15, 17, 48, 49],
        1013: [3, 6, 11, 12, 15, 19], 1014: [3, 6, 7, 8, 13, 15, 17, 18, 49],
        1015: [3, 6, 11, 13, 15, 19, 51, 53],
    }
    assignments = {}
    for key, skill in skills.items():
        if skill['skillId'] in additions:
            assignments[key] = {'variantIds': list(dict.fromkeys(skill['variantIds'] + additions[skill['skillId']]))}
    for filename, data in [('new-variants.json', rows), ('variant-updates.json', updates), ('assignments.json', assignments)]:
        (OUT / filename).write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps({'new_variants': len(rows), 'updated_variants': len(updates), 'assigned_skills': len(assignments)}))


if __name__ == '__main__':
    main()
