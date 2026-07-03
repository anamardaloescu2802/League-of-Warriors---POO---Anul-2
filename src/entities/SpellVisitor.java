package entities;

public class SpellVisitor implements Visitor<Entity> {
    private Spell spell;

    public SpellVisitor(Spell spell) {
        this.spell = spell;
    }

    @Override
    public void visit(Entity entity) {

        if (entity.isImmuneTo(spell.getType())) {
            System.out.println("Entitatea este imună la acest tip de spell!");
        } else {
            System.out.println("entities.Spell " + spell.getType() + " aplicat asupra entității!");
            entity.receiveDamage(spell.getDamage());
        }
    }
}
