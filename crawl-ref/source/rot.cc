/**
 * @file
 * @brief Functions for blood, chunk, & corpse rot.
 **/

#include "AppHdr.h"

#include "rot.h"

#include <algorithm>

#include "areas.h"
#include "butcher.h"
#include "delay.h"
#include "english.h"
#include "env.h"
#include "godconduct.h"
#include "itemprop.h"
#include "items.h"
#include "player-equip.h"
#include "religion.h"
#include "shopping.h"

#define TIMER_KEY "timer"

static bool _is_chunk(const item_def &item);
static bool _item_needs_rot_check(const item_def &item);
//static int _get_initial_stack_longevity(const item_def &stack);

static void _rot_corpse(item_def &it, int mitm_index, int rot_time);
static int _rot_stack(item_def &it, int slot, bool in_inv);

static void _compare_stack_quantity(item_def &stack);

static void _print_chunk_messages(int num_chunks, int num_chunks_gone);

static void _potion_stack_changed_message(string item_name, int num_changed,
                                          int remainder);



/**
 * Checks if a given item can rot.
 *
 * @param stack  The item to check.
 * @return       Whether the given item is either chunks or blood potions.
 */
bool is_perishable_stack(const item_def &item)
{
    return false;
}

/**
 * Initialise a stack of perishable items with a vector of timers, representing
 * the time at which each item in the stack will rot.
 *
 * If the stack has CORPSE_NEVER_DECAYS marked, we won't actually initialize
 * the stack, since we don't want its rot timer to start yet.
 *
 * @param stack     The stack of items to be initialized.
 * @param age       The age for which the stack will last before rotting.
 * (If -1, will be initialized to a default value.)
 */
void init_perishable_stack(item_def &stack, int age)
{
    return;
}

// Compare two CrawlStoreValues storing type T.
template<class T>
static bool _storeval_greater(const CrawlStoreValue &a,
                              const CrawlStoreValue &b)
{
    return T(a) > T(b);
}

// Sort a CrawlVector containing only Ts.
template<class T>
static void _sort_cvec(CrawlVector &vec)
{
    sort(vec.begin(), vec.end(), _storeval_greater<T>);
}

/**
 * Check whether an item decays over time. (Corpses, chunks, and blood.)
 *
 * @param item      The item to check.
 * @return          Whether the item changes (rots) over time.
 */
static bool _item_needs_rot_check(const item_def &item)
{
    if (!item.defined())
        return false;

    if (item.props.exists(CORPSE_NEVER_DECAYS))
        return false;

    if (is_perishable_stack(item))
        return true;

    return item.base_type == OBJ_CORPSES
           && item.sub_type <= CORPSE_SKELETON; // XXX: is this needed?
}

/**
 * Rot a corpse or skeleton lying on the floor.
 *
 * @param it            The corpse or skeleton to rot.
 * @param mitm_index    The slot of the corpse in the floor item array.
 * @param rot_time      The amount of time to rot the corpse for.
 */
static void _rot_corpse(item_def &it, int mitm_index, int rot_time)
{
    return;
}

/**
 * Ensure that a stack of blood potions or chunks has one timer per item in the
 * stack.
 *
 * @param stack         The stack to be potentially initialized.
 * @param timer_size    The # of timers the stack's current props have.
 */
static void _compare_stack_quantity(item_def &stack)
{
  return;
  /*
    CrawlVector &stack_timer = stack.props[TIMER_KEY].get_vector();
    const int timer_size = stack_timer.size();
    if (timer_size != stack.quantity)
    {
        mprf(MSGCH_WARN,
             "ERROR: stack quantity (%d) doesn't match timer (%d)",
             stack.quantity, timer_size);

        // sanity measure; sync stack/timer size

        // more items than timers
	//        const int default_timer = _get_initial_stack_longevity(stack)
        //                          + you.elapsed_time;
        while (stack.quantity > stack_timer.size())
            stack_timer.push_back(default_timer);
        // more timers than items
        while (stack_timer.size() > stack.quantity)
            stack_timer.pop_back();

        ASSERT(stack.quantity == stack_timer.size());
	} */
}

/**
 * Rot a stack of chunks or blood potions.
 *
 * @param it        The stack to rot.
 * @param inv_slot  The slot the item holds. (In mitm or inv.)
 * @param in_inv    Whether the item is in the player's inventory.
 * @return          The number of items rotted away completely.
 */
static int _rot_stack(item_def &it, int slot, bool in_inv)
{
    return 0;
}

/**
 * Decay items on the floor: corpses, chunks, and Gozag gold auras.
 *
 * @param elapsedTime   The amount of time to rot the corpses for.
 */
void rot_floor_items(int elapsedTime)
{
    if (elapsedTime <= 0)
        return;

    const int rot_time = elapsedTime / ROT_TIME_FACTOR;

    for (int mitm_index = 0; mitm_index < MAX_ITEMS; ++mitm_index)
    {
        item_def &it = mitm[mitm_index];

        if (is_shop_item(it) || !_item_needs_rot_check(it))
            continue;

        _rot_stack(it, mitm_index, false);
    }
}


// XXX: unify this with blood messaging somehow?
static void _print_chunk_messages(int num_chunks, int num_chunks_gone)
{
    if (num_chunks_gone > 0)
    {
        mprf(MSGCH_ROTTEN_MEAT,
             "%s of the chunks of flesh in your inventory have rotted away.",
             num_chunks_gone == num_chunks ? "All" : "Some");
    }
}

// Prints messages for blood potions coagulating or rotting in inventory.
static void _potion_stack_changed_message(string item_name, int num_changed,
                                          int initial_quantity)
{
    ASSERT(num_changed > 0);

    mprf(MSGCH_ROTTEN_MEAT, "%s %s rot%s away.",
         get_desc_quantity(num_changed, initial_quantity).c_str(),
         item_name.c_str(),
         num_changed == 1 ? "s" : "");
}

// Removes the oldest timer of a stack of blood potions.
// Mostly used for (q)uaff and (f)ire.
int remove_oldest_perishable_item(item_def &stack)
{
    return 0;
}

// Used whenever copies of blood potions have to be cleaned up.
void remove_newest_perishable_item(item_def &stack, int quant)
{
    return;
}

void merge_perishable_stacks(const item_def &source, item_def &dest, int quant)
{
    return;
}
