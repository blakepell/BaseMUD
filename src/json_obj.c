/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#include <string.h>

#include "db.h"
#include "lookup.h"
#include "utils.h"

#include "json_obj.h"

#define JBITSF(array, bits) \
    JBITS (flag_string ((array), (bits)))

const char *json_not_none (const char *value) {
    return (value == NULL || !strcmp (value, "none"))
        ? NULL : value;
}

const char *json_not_blank (const char *value) {
    return (value == NULL || value[0] == '\0')
        ? NULL : value;
}

JSON_PROP_FUN (obj_room, const ROOM_INDEX_T *);
JSON_T *json_new_obj_room (const char *name, const ROOM_INDEX_T *room) {
    JSON_T *new, *sub;
    EXTRA_DESCR_T *ed;
    RESET_T *r;
    int i;

    if (room == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_ROOM);

    json_prop_string  (new, "area",        room->area->name);
    json_prop_integer (new, "anum",        room->anum);
    json_prop_string  (new, "name",        JSTR (room->name));
    json_prop_string  (new, "description", JSTR (room->description));
    json_prop_string  (new, "sector_type", sector_get_name(room->sector_type));

    if (json_not_blank (room->owner))
        json_prop_string  (new, "owner",       JSTR (room->owner));
    if (room->room_flags != 0)
        json_prop_string  (new, "room_flags",  JBITS (room_bit_name (room->room_flags)));
    if (room->heal_rate != 100)
        json_prop_integer (new, "heal_rate",   room->heal_rate);
    if (room->mana_rate != 100)
        json_prop_integer (new, "mana_rate",   room->mana_rate);
    if (room->clan != 0)
        json_prop_string  (new, "clan",        JSTR (clan_get_name (room->clan)));
    if (room->portal && json_not_blank (room->portal->name))
        json_prop_string (new, "portal", room->portal->name);

    for (i = 0; i < DIR_MAX; i++)
        if (room->exit[i] != NULL)
            break;
    if (i < DIR_MAX) {
        sub = json_prop_array (new, "doors");
        for (i = 0; i < DIR_MAX; i++)
            if (room->exit[i] != NULL)
                json_prop_obj_exit (sub, NULL, room, i, room->exit[i]);
    }

    if (room->extra_descr) {
        sub = json_prop_array (new, "extra_description");
        for (ed = room->extra_descr; ed != NULL; ed = ed->next)
            json_prop_obj_extra_descr (sub, NULL, ed);
    }

    if (room->reset_first) {
        sub = json_prop_array (new, "resets");
        for (r = room->reset_first; r != NULL; r = r->next)
            json_prop_obj_reset (sub, NULL, r);
    }

    return new;
}

JSON_PROP_FUN (obj_extra_descr, const EXTRA_DESCR_T *);
JSON_T *json_new_obj_extra_descr (const char *name, const EXTRA_DESCR_T *ed) {
    JSON_T *new;
    if (ed == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_EXTRA_DESCR);

    json_prop_string (new, "keyword",     ed->keyword);
    json_prop_string (new, "description", ed->description);

    return new;
}

JSON_T *json_prop_obj_exit (JSON_T *parent, const char *name,
    const ROOM_INDEX_T *from, int dir, const EXIT_T *ex)
{
    JSON_T *new = json_new_obj_exit (name, from, dir, ex);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_new_obj_exit (const char *name, const ROOM_INDEX_T *from, int dir,
    const EXIT_T *ex)
{
    JSON_T *new;
    if (ex == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_EXIT);

    json_prop_string  (new, "dir", door_get_name (dir));
    if (ex->to_room == NULL || ex->to_room->area != from->area || ex->portal)
        json_prop_null (new, "to");
    else
        json_prop_integer (new, "to", ex->to_room->anum);

    if (json_not_blank (ex->keyword))
        json_prop_string (new, "keyword",     JSTR (ex->keyword));
    if (json_not_blank (ex->description))
        json_prop_string (new, "description", JSTR (ex->description));

    if (ex->rs_flags != 0)
        json_prop_string (new, "exit_flags", JBITSF (exit_flags, ex->rs_flags));
    if (ex->rs_flags & EX_ISDOOR) {
        if (ex->key == 0)
            json_prop_string (new, "key", "nokey");
        else if (ex->key >= KEY_VALID)
            json_prop_obj_anum (new, "key", from->area, ex->key);
    }
    if (ex->portal && json_not_blank (ex->portal->name))
        json_prop_string  (new, "portal",     ex->portal->name);

    return new;
}

JSON_PROP_FUN (obj_shop, const SHOP_T *);
JSON_T *json_new_obj_shop (const char *name, const SHOP_T *shop) {
    JSON_T *new, *sub;
#if 0
    MOB_INDEX_T *mob;
#endif
    int i;

    if (shop == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SHOP);

#if 0
    mob = get_mob_index (shop->keeper);
    if (mob == NULL)
        json_prop_string (new, "keeper", "unknown");
    else
        json_prop_obj_anum (new, "keeper", shop->area, mob->vnum);
#endif

    sub = json_prop_array (new, "trades");
    for (i = 0; i < MAX_TRADE; i++) {
        const char *name = shop->buy_type[i] <= 0 ? NULL :
            item_get_name (shop->buy_type[i]);
        if (name == NULL)
            continue;
        if (strcmp (name, "unknown") == 0) {
            fprintf (stderr, "json_new_obj_stop(): Unknown item type '%d'.\n",
                shop->buy_type[i]);
            continue;
        }
        json_prop_string (sub, NULL, name);
    }

    json_prop_integer (new, "profit_buy",  shop->profit_buy);
    json_prop_integer (new, "profit_sell", shop->profit_sell);
    json_prop_integer (new, "open_hour",   shop->open_hour);
    json_prop_integer (new, "close_hour",  shop->close_hour);

    return new;
}

JSON_PROP_FUN (obj_mobile, const MOB_INDEX_T *);
JSON_T *json_new_obj_mobile (const char *name, const MOB_INDEX_T *mob) {
    JSON_T *new, *sub;
    const char *spec_fun;
    int i;

    if (mob == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_MOBILE);

    json_prop_string  (new, "area",        mob->area->name);
    json_prop_integer (new, "anum",        mob->anum);
    json_prop_string  (new, "name",        JSTR (mob->name));
    json_prop_string  (new, "short_descr", JSTR (mob->short_descr));
    json_prop_string  (new, "long_descr",  JSTR (mob->long_descr));
    json_prop_string  (new, "description", JSTR (mob->description));
    json_prop_string  (new, "race",        JSTR (race_get_name (mob->race)));

    if (mob->material != MATERIAL_GENERIC)
        json_prop_string  (new, "material", JSTR (material_get_name (mob->material)));

    json_prop_integer (new, "alignment",   mob->alignment);
    json_prop_integer (new, "group",       mob->group);
    json_prop_integer (new, "level",       mob->level);
    json_prop_integer (new, "hitroll",     mob->hitroll);
    json_prop_dice    (new, "hit_dice",    &(mob->hit));
    json_prop_dice    (new, "mana_dice",   &(mob->mana));
    json_prop_dice    (new, "damage_dice", &(mob->damage));
    json_prop_string  (new, "dam_type",    JSTR (attack_get_name (mob->dam_type)));

    sub = json_prop_object (new, "ac", JSON_OBJ_ANY);
    for (i = 0; i < AC_MAX; i++)
        json_prop_integer (sub, ac_types[i].name, mob->ac[ac_types[i].bit]);

    if (mob->start_pos != POS_STANDING)
        json_prop_string (new, "start_pos",
            JSTR (position_get_name (mob->start_pos)));
    if (mob->default_pos != POS_STANDING)
        json_prop_string  (new, "default_pos",
            JSTR (position_get_name (mob->default_pos)));
    if (mob->sex != SEX_EITHER)
        json_prop_string  (new, "sex",
            JSTR (sex_get_name (mob->sex)));

    json_prop_integer (new, "wealth", mob->wealth);
    json_prop_string  (new, "size",   JSTR (size_get_name (mob->size)));
    json_prop_obj_shop(new, "shop",   mob->pShop);

    if (mob->mob_plus != 0)
        json_prop_string (new, "mob_flags",   JBITSF (mob_flags, mob->mob_plus & ~MOB_IS_NPC));
    if (mob->affected_by_plus != 0)
        json_prop_string (new, "affected_by", JBITSF (affect_flags, mob->affected_by_plus));
    if (mob->off_flags_plus != 0)
        json_prop_string (new, "offense",     JBITSF (off_flags, mob->off_flags_plus));
    if (mob->imm_flags_plus != 0)
        json_prop_string (new, "immune",      JBITSF (res_flags, mob->imm_flags_plus));
    if (mob->res_flags_plus != 0)
        json_prop_string (new, "resist",      JBITSF (res_flags, mob->res_flags_plus));
    if (mob->vuln_flags_plus != 0)
        json_prop_string (new, "vulnerable",  JBITSF (res_flags, mob->vuln_flags_plus));
    if (mob->form_plus != 0)
        json_prop_string (new, "form",        JBITSF (form_flags, mob->form_plus));
    if (mob->parts_plus != 0)
        json_prop_string (new, "parts",       JBITSF (part_flags, mob->parts_plus));

    if (mob->mob_minus != 0)
        json_prop_string (new, "mob_flags_minus",   JBITSF (mob_flags, mob->mob_minus));
    if (mob->affected_by_minus != 0)
        json_prop_string (new, "affected_by_minus", JBITSF (affect_flags, mob->affected_by_minus));
    if (mob->off_flags_minus != 0)
        json_prop_string (new, "offense_minus",     JBITSF (off_flags, mob->off_flags_minus));
    if (mob->imm_flags_minus != 0)
        json_prop_string (new, "immune_minus",      JBITSF (res_flags, mob->imm_flags_minus));
    if (mob->res_flags_minus != 0)
        json_prop_string (new, "resist_minus",      JBITSF (res_flags, mob->res_flags_minus));
    if (mob->vuln_flags_minus != 0)
        json_prop_string (new, "vulnerable_minus",  JBITSF (res_flags, mob->vuln_flags_minus));
    if (mob->form_minus != 0)
        json_prop_string (new, "form_minus",        JBITSF (form_flags, mob->form_minus));
    if (mob->parts_minus != 0)
        json_prop_string (new, "parts_minus",       JBITSF (part_flags, mob->parts_minus));

    if (mob->mprogs)
        printf ("*** Ignoring '%s' (#%d) mprogs ***\n", mob->short_descr, mob->vnum);

    spec_fun = spec_function_name (mob->spec_fun);
    json_prop_string (new, "spec_fun", JSTR (spec_fun));

    return new;
}

JSON_PROP_FUN (obj_object, const OBJ_INDEX_T *);
JSON_T *json_new_obj_object (const char *name, const OBJ_INDEX_T *obj) {
    JSON_T *new, *sub;
    const OBJ_MAP_T *map;
    EXTRA_DESCR_T *ed;
    AFFECT_T *aff;

    if (obj == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_string  (new, "area",        obj->area->name);
    json_prop_integer (new, "anum",        obj->anum);
    json_prop_string  (new, "name",        obj->name);
    json_prop_string  (new, "short_descr", JSTR (obj->short_descr));
    json_prop_string  (new, "description", JSTR (obj->description));
    if (obj->material != MATERIAL_GENERIC)
        json_prop_string (new, "material", JSTR (material_get_name (obj->material)));
    json_prop_string  (new, "item_type",   JBITS (item_get_name (obj->item_type)));

    sub = json_prop_object (new, "values", JSON_OBJ_ANY);
    map = obj_map_get (obj->item_type);
    if (map == NULL) {
        bugf ("json_new_obj_object: No map for object '%s' (%d).",
            item_get_name (obj->item_type), obj->item_type);
    }
    else {
        const OBJ_MAP_VALUE_T *value;
        JSON_T *vjson;
        flag_t vobj;
        int i;

        for (i = 0; i < OBJ_VALUE_MAX; i++) {
            if ((value = obj_map_value_get (map, i)) == NULL)
                continue;

            vjson = NULL;
            vobj = obj->v.value[i];

            if (vobj == value->default_value)
                continue;

            switch (value->type) {
                case MAP_INTEGER:
                    vjson = json_new_integer (value->name, vobj);
                    break;
                case MAP_BOOLEAN:
                    vjson = json_new_boolean (value->name, vobj);
                    break;
                case MAP_IGNORE: {
                    char vname[16];
                    snprintf (vname, sizeof (vname), "value%d", i);
                    vjson = json_new_integer (vname, vobj);
                    break;
                }

                case MAP_LOOKUP: {
                    const char *vstr = map_lookup_get_string (
                        value->sub_type, vobj);
                    if (vstr == NULL)
                        vjson = json_new_null (value->name);
                    else
                        vjson = json_new_string (value->name,
                            (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                case MAP_FLAGS: {
                    char vstr[256];
                    if (!map_flags_get_string (value->sub_type, vobj, vstr, sizeof(vstr)))
                        vjson = json_new_null (value->name);
                    else
                        vjson = json_new_string (value->name,
                            (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                default:
                    bugf ("json_new_obj_object: Cannot convert map type '%d' "
                          "to JSON value", value->type);
                    continue;
            }

            if (vjson)
                json_attach_under (vjson, sub);
        }
    }

    json_prop_integer (new, "level",     obj->level);
    json_prop_integer (new, "weight",    obj->weight);
    json_prop_integer (new, "cost",      obj->cost);
    json_prop_integer (new, "condition", obj->condition);

    if (obj->extra_flags != 0)
        json_prop_string  (new, "extra_flags", JBITS (extra_bit_name (obj->extra_flags)));
    if (obj->wear_flags != 0)
        json_prop_string  (new, "wear_flags",  JBITS (wear_flag_name (obj->wear_flags)));

    if (obj->extra_descr) {
        sub = json_prop_array (new, "extra_description");
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
            json_prop_obj_extra_descr (sub, NULL, ed);
    }

    if (obj->affected) {
        sub = json_prop_array (new, "affects");
        for (aff = obj->affected; aff != NULL; aff = aff->next)
            json_prop_obj_affect (sub, NULL, aff);
    }

    return new;
}

JSON_PROP_FUN (obj_affect, const AFFECT_T *);
JSON_T *json_new_obj_affect (const char *name, const AFFECT_T *aff) {
    JSON_T *new;
    const char *bv;
    const AFFECT_BIT_T *bits;

    if (aff == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_string  (new, "apply",    flag_string (affect_apply_types, aff->apply));
    json_prop_integer (new, "level",    aff->level);
    json_prop_integer (new, "modifier", aff->modifier);
    json_prop_string  (new, "bit_type", affect_bit_get_name (aff->bit_type));

    bv = NULL;
    bits = affect_bit_get (aff->bit_type);
    if (bits == NULL)
        bugf ("json_new_obj_affect: Unhandled bit_type '%d'", aff->bit_type);
    else {
        bv = JBITSF (bits->flags, aff->bits);
        if (bv != NULL && bv[0] == '\0')
            bv = NULL;
    }

    json_prop_string (new, "bits", bv);
    return new;
}

JSON_T *json_prop_obj_anum (JSON_T *parent, const char *name,
    AREA_T *area_from, int vnum)
{
    JSON_T *new = json_new_obj_anum (name, area_from, vnum);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_new_obj_anum (const char *name, AREA_T *area_from, int vnum) {
    if (vnum >= area_from->min_vnum && vnum <= area_from->max_vnum) {
        return json_new_integer (name, vnum - area_from->min_vnum);
    }
    else {
        const AREA_T *area;
        JSON_T *new;

        if ((area = area_get_by_inner_vnum (vnum)) == NULL) {
            fprintf (stderr, "json_new_obj_anum(): No area for "
                "vnum '%d'\n", vnum);
            return json_new_null (name);
        }

        new = json_new_object (name, JSON_OBJ_ANY);
        json_prop_string  (new, "area", JSTR (area->name));
        json_prop_integer (new, "anum", vnum - area->min_vnum);
        return new;
    }
}

JSON_PROP_FUN (obj_reset, const RESET_T *);
JSON_T *json_new_obj_reset (const char *name, const RESET_T *reset) {
    JSON_T *new, *sub;
    const RESET_VALUE_T *v;
    char *command, letter_buf[2];

    if (reset == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_RESET);

    switch (reset->command) {
        case 'M': command = "mobile";    break;
        case 'O': command = "object";    break;
        case 'G': command = "give";      break;
        case 'E': command = "equip";     break;
        case 'P': command = "put";       break;
        case 'R': command = "randomize"; break;

        default:
            letter_buf[0] = reset->command;
            letter_buf[1] = '\0';
            command = letter_buf;
            break;
    }
    json_prop_string (new, "command", command);

    sub = json_prop_object (new, "values", JSON_OBJ_ANY);
    v = &(reset->v);
    switch (reset->command) {
        case 'M':
            json_prop_obj_anum (sub, "mob", reset->area, v->mob.mob_vnum);
            json_prop_integer  (sub, "global_limit", v->mob.global_limit);
            json_prop_integer  (sub, "room_limit",   v->mob.room_limit);
            break;

        case 'O':
            json_prop_obj_anum (sub, "obj", reset->area, v->obj.obj_vnum);
            json_prop_integer  (sub, "global_limit", v->obj.global_limit);
            json_prop_integer  (sub, "room_limit",   v->obj.room_limit);
            break;

        case 'G':
            json_prop_obj_anum (sub, "obj", reset->area, v->give.obj_vnum);
            json_prop_integer  (sub, "global_limit", v->give.global_limit);
            break;

        case 'E':
            json_prop_obj_anum (sub, "obj", reset->area, v->equip.obj_vnum);
            json_prop_string   (sub, "wear_loc", flag_string (wear_loc_types,
                v->equip.wear_loc));
            json_prop_integer  (sub, "global_limit", v->equip.global_limit);
            break;

        case 'P':
            json_prop_obj_anum (sub, "obj",  reset->area, v->put.obj_vnum);
            json_prop_obj_anum (sub, "into", reset->area, v->put.into_vnum);
            json_prop_integer  (sub, "global_limit", v->put.global_limit);
            json_prop_integer  (sub, "put_count",    v->put.put_count);
            break;

        case 'R':
            json_prop_integer (sub, "dir_count", v->randomize.dir_count);
            break;

        default:
            bugf ("json_new_obj_reset: Unhandled command '%c'", reset->command);
            json_prop_integer (sub, "value0", reset->v.value[0]);
            json_prop_integer (sub, "value1", reset->v.value[1]);
            json_prop_integer (sub, "value2", reset->v.value[2]);
            json_prop_integer (sub, "value3", reset->v.value[3]);
            json_prop_integer (sub, "value4", reset->v.value[4]);
    }

    return new;
}

JSON_PROP_FUN (obj_area, const AREA_T *);
JSON_T *json_new_obj_area (const char *name, const AREA_T *area) {
    JSON_T *new;
    if (area == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_AREA);

    json_prop_string  (new, "name",       area->name);
    json_prop_string  (new, "filename",   area->filename);
    json_prop_string  (new, "title",      area->title);
    json_prop_string  (new, "credits",    area->credits);
    json_prop_integer (new, "min_vnum",   area->min_vnum);
    json_prop_integer (new, "max_vnum",   area->max_vnum);
    json_prop_string  (new, "builders",   area->builders);
    json_prop_integer (new, "security",   area->security);

    if (area->low_range != 0)
        json_prop_integer (new, "low_range",  area->low_range);
    if (area->high_range != 0)
        json_prop_integer (new, "high_range", area->high_range);

    return new;
}

JSON_PROP_FUN (obj_social, const SOCIAL_T *);
JSON_T *json_new_obj_social (const char *name, const SOCIAL_T *soc) {
    JSON_T *new;
    if (soc == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SOCIAL);

    json_prop_string (new, "name",           soc->name);
    if (soc->char_no_arg)
        json_prop_string (new, "char_no_arg",    soc->char_no_arg);
    if (soc->others_no_arg)
        json_prop_string (new, "others_no_arg",  soc->others_no_arg);
    if (soc->char_found)
        json_prop_string (new, "char_found",     soc->char_found);
    if (soc->others_found)
        json_prop_string (new, "others_found",   soc->others_found);
    if (soc->vict_found)
        json_prop_string (new, "vict_found",     soc->vict_found);
    if (soc->char_not_found)
        json_prop_string (new, "char_not_found", soc->char_not_found);
    if (soc->char_auto)
        json_prop_string (new, "char_auto",      soc->char_auto);
    if (soc->others_auto)
        json_prop_string (new, "others_auto",    soc->others_auto);

    return new;
}

JSON_PROP_FUN (obj_table, const TABLE_T *);
JSON_T *json_new_obj_table (const char *name, const TABLE_T *table) {
    JSON_T *new, *sub;
    char *type_str;

    if (table == NULL || table->json_write_func == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_TABLE);

    type_str = NULL;
    if (table->flags & TABLE_FLAG_TYPE)
        type_str = (table->flags & TABLE_BITS) ? "flags" : "types";
    else
        type_str = "table";

    json_prop_string (new, "name",        table->name);
    json_prop_string (new, "description", table->description);
    json_prop_string (new, "type",        type_str);

    sub = json_prop_array (new, "values");

    {
        const void *obj = table->table;
        const FLAG_T *flag;
        JSON_T *json;
        flag_t expected = (table->flags & TABLE_BITS) ? 1 : 0;
        bool had_none = FALSE;

        do {
            if ((json = table->json_write_func (obj)) == NULL)
                break;
            if (table->flags & TABLE_FLAG_TYPE) {
                flag = obj;
                if (had_none) {
                    bugf ("json_new_obj_table: Warning: %s table '%s' row "
                        "'%s' should not be after -1 bit", type_str,
                        table->name, flag->name);
                }
                else if (flag->bit == -1 && !IS_SET(table->flags, TABLE_BITS))
                    had_none = TRUE;
                else if (flag->bit != expected) {
                    bugf ("json_new_obj_table: Warning: %s table '%s' row "
                        "'%s' should be %ld, but it's %ld", type_str,
                        table->name, flag->name, expected, flag->bit);
                }

                if (table->flags & TABLE_BITS)
                    expected <<= 1;
                else
                    ++expected;
            }

            json_attach_under (json, sub);
            obj += table->type_size;
        } while (1);
    }
    return new;
}

JSON_PROP_FUN (obj_portal_exit, const PORTAL_EXIT_T *);
JSON_T *json_new_obj_portal_exit (const char *name, const PORTAL_EXIT_T *pex) {
    JSON_T *new;
    if (pex == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL_EXIT);

    return new;
}

JSON_PROP_FUN (obj_portal, const PORTAL_T *);
JSON_T *json_new_obj_portal (const char *name, const PORTAL_T *portal) {
    JSON_T *new;
    if (portal == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL);

    json_prop_boolean (new, "two-way", portal->two_way);
    json_prop_string  (new, "from",    portal->name_from);
    json_prop_string  (new, "to",      portal->name_to);

    return new;
}

JSON_PROP_FUN (obj_help_area, const HELP_AREA_T *);
JSON_T *json_new_obj_help_area (const char *name, const HELP_AREA_T *had) {
    JSON_T *new, *sub;
    HELP_T *help;
    if (had == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP_AREA);

    if (had->area)
        json_prop_string (new, "area", JSTR (had->area->name));
    else
        json_prop_string (new, "area", NULL);

    json_prop_string (new, "name",     JSTR (had->name));
    json_prop_string (new, "filename", JSTR (had->filename));

    sub = json_prop_array (new, "pages");
    for (help = had->first; help; help = help->next_area)
        json_prop_obj_help (sub, NULL, help);

    return new;
}

JSON_PROP_FUN (obj_help, const HELP_T *);
JSON_T *json_new_obj_help (const char *name, const HELP_T *help) {
    JSON_T *new;
    if (help == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP);

    json_prop_string  (new, "keyword", JSTR (help->keyword));
    json_prop_integer (new, "level",   help->level);
    json_prop_string  (new, "text",    JSTR (help->text));

    return new;
}
