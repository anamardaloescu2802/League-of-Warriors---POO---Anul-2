package entities.characters;

public class Warrior extends Character{

    public Warrior(String name, int level, int experience) {
        super(name, level, experience, 120, 60, 5, 10, 20);
    }

    @Override
    public int calculateDamage() {
        int damage = getStrength() * 3;
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
