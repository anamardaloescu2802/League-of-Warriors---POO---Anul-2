package entities.characters;

public class Mage extends Character {
    public Mage(String name, int level, int experience) {
        super(name, level, experience, 80, 120, 15, 10, 5);  }

    @Override
    public int calculateDamage() {
        int damage = getCharisma() * 2;
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
