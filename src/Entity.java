import entities.Battle;
import entities.Spell;

import java.util.ArrayList;

public abstract class Entity implements Battle, Element<Entity> {
    private int currentHealth;
    private int maxHealth;
    private int currentMana;
    private int maxMana;
    private ArrayList<Spell> abilities;
    private boolean immuneToFire;
    private boolean immuneToIce;
    private boolean immuneToEarth;

    public Entity(int health, int mana) {
        this.currentHealth = health;
        this.maxHealth = health;
        this.currentMana = mana;
        this.maxMana = mana;
        this.abilities = new ArrayList<>();
        this.immuneToFire = false;
        this.immuneToIce = false;
        this.immuneToEarth = false;
    }

    public int getCurrentHealth() {
        return currentHealth;
    }

    public int getCurrentMana() {
        return currentMana;
    }

    public ArrayList<Spell> getAbilities() {
        return abilities;
    }

    public void setImmunities(boolean fire, boolean ice, boolean earth) {
        this.immuneToFire = fire;
        this.immuneToIce = ice;
        this.immuneToEarth = earth;
    }

    public void regenerateHealth(int amount) {
        currentHealth = Math.min(currentHealth + amount, maxHealth);
        System.out.println("Viata regenerata: " + amount + "| Viata curenta: " + currentHealth);
    }

    public void regenerateMana(int amount) {
        currentMana = Math.min(currentMana + amount, maxMana);
        System.out.println("Mana regenerata: " + amount + "| Mana curenta: " + currentMana);
    }

    public void useAbility(Spell spell, Entity enemy) {
        if (currentMana >= spell.getManaCost()) {
            currentMana -= spell.getManaCost();
            System.out.println("Abilitatea folosita: " + spell.getType());
            System.out.println("Damage: " + spell.getDamage() + ", Mana cosumata: " + spell.getManaCost());

            if (enemy.isImmuneTo(spell.getType())) {
                System.out.println("Inamic imun la" + spell.getType());
            } else {
                enemy.receiveDamage(spell.getDamage());
                System.out.println("Inamicul a primit " + spell.getDamage() + " damage.");
            }
        } else {
            System.out.println("Nu ai destula mana!" + spell.getType());
        }
    }

    public boolean isImmuneTo(String type) {
        switch (type) {
            case "Fire":
                return immuneToFire;
            case "Ice":
                return immuneToIce;
            case "Earth":
                return immuneToEarth;
            default:
                return false;
        }
    }

    @Override
    public void receiveDamage(int damage) {
        currentHealth -= damage;
        if (currentHealth < 0) {
            currentHealth = 0;
        }
        System.out.println("Damage primit " + damage + "| Viata curenta: " + currentHealth);
    }

    @Override
    public int getDamage() {
        return 10;
    }

    public void addAbility(Spell spell) {
        abilities.add(spell);
    }

    @Override
    public void accept(Visitor<Entity> visitor) {
        visitor.visit(this);  // Permite unui visitor să aplice efectul asupra entității
    }
}
