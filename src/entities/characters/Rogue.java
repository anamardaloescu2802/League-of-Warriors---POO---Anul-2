package entities.characters;

public class Rogue extends Character {
    public Rogue(String name, int level, int experience) {
        super(name, level, experience, 90, 80, 5, 20, 10); // Exemplu: valori implicite pentru health, mana, charisma, dexterity, strength
    }

    @Override
    public int calculateDamage() {
         int damage = getDexterity() * 2;
        if (Math.random() < 0.5) {
            damage *= 2;
        }
        return damage;
    }
    @Override
    public void levelUp() {
        super.levelUp();
    }
}

