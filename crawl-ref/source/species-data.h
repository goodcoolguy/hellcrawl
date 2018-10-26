enum species_flag
{
    SPF_NONE        = 0,
    SPF_ELVEN       = 1 << 0, /// If this species counts as an elf
    SPF_DRACONIAN   = 1 << 1, /// If this is a draconian subspecies
    SPF_ORCISH      = 1 << 2, /// If this species is a kind of orc
    SPF_NO_HAIR     = 1 << 3, /// If members of the species are hairless
    SPF_SMALL_TORSO = 1 << 4, /// Torso is smaller than body
};
DEF_BITFIELD(species_flags, species_flag);

struct level_up_mutation
{
    mutation_type mut; ///< What mutation to give
    int mut_level; ///< How much to give
    int xp_level; ///< When to give it (if 1, is a starting mutation)
};

struct species_def
{
  const char* abbrev; ///< Two-letter abbreviation
  const char* name; ///< Main name
  const char* adj_name; ///< Adjectival form of name; if null, use name
  const char* genus_name; ///< Genus name; if null, use name
  species_flags flags; ///< Miscellaneous flags
    // The following three need to be 2 lines after the name for gen-apt.pl:
    int xp_mod; ///< Experience level modifier
    int hp_mod; ///< HP modifier (in tenths)
    int mp_mod; ///< MP modifier
    int mr_mod; ///< MR modifier (multiplied by XL for base MR)
    monster_type monster_species; //< Corresponding monster (for display)
    habitat_type habitat; //< Where it can live; HT_WATER -> no penalties
    undead_state_type undeadness; ///< What kind of undead (if any)
    size_type size; ///< Size of body
    map<stat_type, int> initial_stats; // stat contributions at character creation
    vector<stat_type> stat_schedule;  // stat distribution on level-up -- should have 14 entries
  // int m, r, d, s, b, e, t; ///< Starting stats contribution
  //set<stat_type> level_stats; ///< Which stats to gain on level-up
  //int how_often; ///< When to level-up stats
    vector<level_up_mutation> level_up_mutations; ///< Mutations on level-up
    vector<string> verbose_fake_mutations; ///< Additional information on 'A'
    vector<string> terse_fake_mutations; ///< Additional information on '%'
  //    vector<job_type> recommended_jobs; ///< Which jobs are "good" for it
  //  vector<skill_type> recommended_weapons; ///< Which weapons types are "good"
};

static const map<species_type, species_def> species_data =
{
  { SP_HUMAN, {
      "Hu",
      "Human", "Human", "Human",
      SPF_NONE,
       0,0,0,0,
       MONS_HUMAN,
       HT_LAND, US_ALIVE, SIZE_MEDIUM,
       {{STAT_CLASS_A, 3}, {STAT_CLASS_B, 2}, {STAT_CLASS_C, 1}},
        {STAT_CLASS_C, // 2
         STAT_CLASS_B, // 3
         STAT_CLASS_A, // 4
         STAT_CLASS_B, // 5
         STAT_CLASS_A, // 6
         STAT_CLASS_C, // 7
         STAT_CLASS_A, // 8
         STAT_CLASS_B, // 9
         STAT_CLASS_A, // 10
         STAT_CLASS_C, // 11
         STAT_CLASS_A, // 12
         STAT_CLASS_B, // 13
         STAT_CLASS_A, // 14
         STAT_CLASS_C },// 15 
        {},
       {},
       {},
    } },

// Ideally this wouldn't be necessary...
{ SP_UNKNOWN, { // Line 1: enum
    "??", // Line 2: abbrev
    "Yak", nullptr, nullptr, // Line 3: name, genus name, adjectival name
    SPF_NONE, // Line 4: flags
    0, 0, 0, 0, // Line 5: XP, HP, MP, MR (gen-apt.pl needs them here!)
    MONS_PROGRAM_BUG, // Line 6: equivalent monster type
    HT_LAND, US_ALIVE, SIZE_MEDIUM, // Line 7: habitat, life, size
    {{STAT_CLASS_A, 1}}, // initial stat/skills
    {}, // stat/skill schedule
    {}, // Line 10: Mutations
    {}, // Line 11: Fake mutations
    {}, // Line 12: Fake mutations
      //{}, // Line 13: Recommended jobs
      //{}, // Line 14: Recommended weapons
} }
};
