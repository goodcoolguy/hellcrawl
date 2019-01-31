/**
 * @file
 * @brief Initializing non-player-related parts of a new game.
**/
/* TODO: 'you' shouldn't occur here.
 *       Some of these might fit better elsewhere.
 */

#include "AppHdr.h"

#include "ng-init.h"

#include "branch.h"
#include "describe.h"
#include "dungeon.h"
#include "end.h"
#include "itemname.h"
#include "libutil.h"
#include "maps.h"
#include "random.h"
#include "religion.h"
#include "spl-util.h"
#include "state.h"
#include "stringutil.h"
#include "unicode.h"

#ifdef DEBUG_DIAGNOSTICS
#define DEBUG_TEMPLES
#endif

static uint8_t _random_potion_description()
{
    int desc;

    desc = random2(PDQ_NQUALS * PDC_NCOLOURS);

    if (coinflip())
        desc %= PDC_NCOLOURS;

    // nature and colour correspond to primary and secondary in
    // itemname.cc.

#if TAG_MAJOR_VERSION == 34
    if (PCOLOUR(desc) == PDC_CLEAR) // only water can be clear, re-roll
        return _random_potion_description();
#endif

    return desc;
}

// Determine starting depths of branches.
void initialise_branch_depths()
{
    root_branch = BRANCH_DUNGEON;

    // XXX: Should this go elsewhere?
    branch_bribe.init(0);

    for (branch_iterator it; it; ++it)
        brentry[it->id].clear();

    if (crawl_state.game_is_sprint())
    {
        brdepth.init(-1);
        brdepth[BRANCH_DUNGEON] = 1;
        return;
    }

    for (int branch = 0; branch < NUM_BRANCHES; ++branch)
    {
        const Branch *b = &branches[branch];
        ASSERT(b->id == branch);
    }

    for (branch_iterator it; it; ++it)
    {
        if (!branch_is_unfinished(it->id) && it->parent_branch != NUM_BRANCHES)
        {
            brentry[it->id] = level_id(it->parent_branch,
                                       random_range(it->mindepth,
                                                    it->maxdepth));
        }
    }

    // You will get one of Shoals/Swamp/Spider/Snake.
    branch_type lair_branch[] =
    {
        BRANCH_SWAMP, 
		BRANCH_SHOALS,
        BRANCH_SNAKE,  	
		BRANCH_SPIDER
    };
	
	branch_type enabled_branch = random_choose(BRANCH_SWAMP, BRANCH_SHOALS, 
	BRANCH_SNAKE, BRANCH_SPIDER);
	
    for (branch_type disabled : lair_branch)
    {
		if(branches[disabled].shortname != branches[enabled_branch].shortname)
		{
        dprf("Disabling branch: %s", branches[disabled].shortname);
        brentry[disabled].clear();
		}
    }
	
	 branch_type hell_branch[] =
    {
        BRANCH_DIS, 
		BRANCH_TARTARUS,
        BRANCH_COCYTUS,  	
		BRANCH_GEHENNA
    };
	
	branch_type enabled_hell = random_choose(BRANCH_DIS, BRANCH_TARTARUS, 
	BRANCH_COCYTUS, BRANCH_GEHENNA);
	
	for (branch_type disabled : hell_branch)
    {
		if(branches[disabled].shortname != branches[enabled_hell].shortname)
		{
        dprf("Disabling branch: %s", branches[disabled].shortname);
        brentry[disabled].clear();
		}
    }

    for (branch_iterator it; it; ++it)
        brdepth[it->id] = it->numlevels;
}

#define MAX_OVERFLOW_LEVEL 7

static void _use_overflow_temple(vector<god_type> temple_gods)
{
    CrawlVector &overflow_temples
        = you.props[OVERFLOW_TEMPLES_KEY].get_vector();

    const unsigned int level = random_range(2, MAX_OVERFLOW_LEVEL);

    // List of overflow temples on this level.
    CrawlVector &level_temples = overflow_temples[level - 1].get_vector();

    CrawlHashTable temple;

    level_temples.push_back(temple);
}

// Determine which altars go into the Ecumenical Temple, which go into
// overflow temples, and on what level the overflow temples are.
void initialise_temples()
{
    ///////////////////////////////////
    // Now set up the overflow temples.

    vector<god_type> god_list = temple_god_list();
    shuffle_array(god_list);

#ifdef DEBUG_TEMPLES
    mprf(MSGCH_DIAGNOSTICS, "%u overflow altars", (unsigned int)god_list.size());
#endif

    CrawlVector &overflow_temples
        = you.props[OVERFLOW_TEMPLES_KEY].new_vector(SV_VEC);
    overflow_temples.resize(MAX_OVERFLOW_LEVEL);

    // Count god overflow temple weights.
    int overflow_weights[NUM_GODS + 1];
    overflow_weights[0] = 0;

    for (unsigned int i = 1; i < NUM_GODS; i++)
    {
        string mapname = make_stringf("temple_overflow_generic_%d", i);
        mapref_vector maps = find_maps_for_tag(mapname);
        if (!maps.empty())
        {
            int chance = 0;
            for (auto map : maps)
            {
                // XXX: this should handle level depth better
                chance += map->weight(level_id(BRANCH_DUNGEON,
                                               MAX_OVERFLOW_LEVEL));
            }
            overflow_weights[i] = chance;
        }
        else
            overflow_weights[i] = 0;
    }

    // NOTE: The overflow temples don't have to contain only one
    // altar; they can contain any number of altars, so long as there's
    // at least one vault definition with the tag "overflow_temple_num"
    // (where "num" is the number of altars).
    for (unsigned int i = 0, size = god_list.size(); i < size; i++)
    {
        unsigned int remaining_size = size - i;
        // At least one god.
        vector<god_type> this_temple_gods;
        this_temple_gods.push_back(god_list[i]);

        // Maybe place a larger overflow temple.
        if (remaining_size > 1 && one_chance_in(remaining_size + 1))
        {
            vector<pair<unsigned int, int> > num_weights;
            unsigned int num_gods = 1;

            // Randomly choose from the sizes which have maps.
            for (unsigned int j = 2; j <= remaining_size; j++)
                if (overflow_weights[j] > 0)
                    num_weights.emplace_back(j, overflow_weights[j]);

            if (!num_weights.empty())
                num_gods = *(random_choose_weighted(num_weights));

            // Add any extra gods (the first was added already).
            for (; num_gods > 1; i++, num_gods--)
                this_temple_gods.push_back(god_list[i + 1]);
        }

        _use_overflow_temple(this_temple_gods);
    }
}

void initialise_item_descriptions()
{
    // Must remember to check for already existing colours/combinations.
    you.item_description.init(255);

    // The order here must match that of IDESC in describe.h
    const int max_item_number[6] = { NUM_WANDS,
                                     NUM_POTIONS,
                                     NUM_SCROLLS,
                                     NUM_JEWELLERY,
                                     NUM_SCROLLS,
                                     NUM_STAVES };

    for (int i = 0; i < NUM_IDESC; i++)
    {
        // Only loop until NUM_WANDS etc.
        for (int j = 0; j < max_item_number[i]; j++)
        {
            // Don't override predefines
            if (you.item_description[i][j] != 255)
                continue;

            // Pick a new description until it's good.
            while (true)
            {
                switch (i)
                {
                case IDESC_WANDS: // wands
                    you.item_description[i][j] = random2(NDSC_WAND_PRI
                                                         * NDSC_WAND_SEC);
                    if (coinflip())
                        you.item_description[i][j] %= NDSC_WAND_PRI;
                    break;

                case IDESC_POTIONS: // potions
                    you.item_description[i][j] = _random_potion_description();
                    break;


#if TAG_MAJOR_VERSION == 34
                case IDESC_SCROLLS_II: // unused but validated
#endif
                case IDESC_SCROLLS: // scrolls: random seed for the name
                {
                    // this is very weird and probably a linleyism.
                    const int seed_1 = random2(151); // why 151?
                    const int seed_2 = random2(151);
                    const int seed_3 = OBJ_SCROLLS; // yes, really
                    you.item_description[i][j] =   seed_1
                                                | (seed_2 << 8)
                                                | (seed_3 << 16);
                    break;
                }

                case IDESC_RINGS: // rings and amulets
                    you.item_description[i][j] = random2(NDSC_JEWEL_PRI
                                                         * NDSC_JEWEL_SEC);
                    if (coinflip())
                        you.item_description[i][j] %= NDSC_JEWEL_PRI;
                    break;

                case IDESC_STAVES: // staves
                    you.item_description[i][j] = random2(NDSC_STAVE_PRI
                                                         * NDSC_STAVE_SEC);
                    break;
                }

                bool is_ok = true;

                // Test whether we've used this description before.
                // Don't have p < j because some are preassigned.
                for (int p = 0; p < max_item_number[i]; p++)
                {
                    if (p == j)
                        continue;

                    if (you.item_description[i][p] == you.item_description[i][j])
                    {
                        is_ok = false;
                        break;
                    }
                }
                if (is_ok)
                    break;
            }
        }
    }
}

void fix_up_jiyva_name()
{
    you.jiyva_second_name = make_name(get_uint32(), MNAME_JIYVA);
    ASSERT(you.jiyva_second_name[0] == 'J');
}
