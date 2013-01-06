/* Prototypes for interface.c, since they didn't exist anywhere else. */
extern void set_speed(int speed);
extern void set_course(unsigned char dir);

extern void shield_up(void);
extern void shield_down(void);

extern void shield_tog(void);
extern void bomb_planet(void);

extern void beam_up(void);
extern void beam_down(void);

extern void repair(void);
extern void repair_off(void);

extern void cloak(void);
extern void cloak_on(void);
extern void cloak_off(void);



extern void borg_init(void);
extern void borg_toggle(void);
extern void borg_update(struct player *me, struct torp *torps, struct plasmatorp *plasma,
            void (*set_heading)(unsigned char),
            void (*set_speed)(int),
            void (*shield_up)(void),
            void (*shield_down)(void),
            void (*repair_start)(void),
            void (*repair_stop)(void));

