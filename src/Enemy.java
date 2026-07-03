import entities.Spell;

import java.util.Random;

    public class Enemy extends Entity {
        private static final Random random = new Random();

        private String type;

        public Enemy() {

            super(random.nextInt(51) + 50, random.nextInt(31) + 20);

            boolean fireImmune = random.nextBoolean();
            boolean iceImmune = random.nextBoolean();
            boolean earthImmune = random.nextBoolean();
            setImmunities(fireImmune, iceImmune, earthImmune);

            int numAbilities = random.nextInt(3) + 4;
            for (int i = 0; i < numAbilities; i++) {
                addAbility(generateRandomSpell());
            }
            int enemyType = random.nextInt(3); // 0 Warrior, 1 Mage, 2 Rogue

            if (enemyType == 0) {
                this.type = "Warrior";
            } else if (enemyType == 1) {
                this.type = "Mage";
            } else if (enemyType == 2) {
                this.type = "Rogue";
            } else {
                throw new IllegalStateException("Valoare Necunoscuta: " + enemyType);
            }
        }

        public String getType() {
            return this.type;
        }

        private Spell generateRandomSpell() {
            int damage = random.nextInt(21) + 10;
            int manaCost = random.nextInt(11) + 5;

            int spellType = random.nextInt(3); // 0 Fire, 1 Ice, 2 Earth
            if (spellType == 0) {
                return new Fire(damage, manaCost);
            } else if (spellType == 1) {
                return new Ice(damage, manaCost);
            } else if (spellType == 2) {
                return new Earth(damage, manaCost);
            } else {
                throw new IllegalStateException("Valoare necunoscuta: " + spellType);
            }

        }

        @Override
        public void receiveDamage(int damage) {
            if (random.nextBoolean()) {
                System.out.println("Inamicul a evitat atacul!");
                return;
            }
            super.receiveDamage(damage);
        }

        @Override
        public int getDamage() {
            int damage = super.getDamage();
            if (random.nextBoolean()) {
                System.out.println("Enemy dealt double damage!");
                return damage * 2;
            }
            return damage;
        }

        @Override
        public String toString() {
            return "\nSanatate " + getCurrentHealth() + "| Mana " + getCurrentMana() + "\nImunitati\nFoc " + isImmuneTo("Fire") +
                    "\nGheata " + isImmuneTo("Ice") + "\nPamant " + isImmuneTo("Earth") + "\nAbilitati=" + getAbilities();
        }
    }
