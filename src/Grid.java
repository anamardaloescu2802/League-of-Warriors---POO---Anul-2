import entities.characters.Character;
import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.Random;

public class Grid extends ArrayList<ArrayList<Cell>> {
    private int length;
    private int width;
    public int currentX;
    public int currentY;
    public Character playerCharacter;
    public int mana;
    public int health;
    public int experience;
    private GameGUI gameGUI;

    public Grid(int length, int width, Character character, GameGUI gameGUI) {
        this.length = length;
        this.width = width;
        this.playerCharacter = character;
        this.health = character.getHealth();
        this.mana = character.getMana();
        this.experience = character.getExperience();
        this.gameGUI = gameGUI;

        for (int i = 0; i < length; i++) {
            ArrayList<Cell> row = new ArrayList<>();
            for (int j = 0; j < width; j++) {
                row.add(new Cell(i, j, CellEntityType.N));
            }
            this.add(row);
        }
    }

    public Cell getCell(int x, int y) {
        if (x >= 0 && x < length && y >= 0 && y < width) {
            return this.get(x).get(y);
        }
        throw new IndexOutOfBoundsException("Coordonatele sunt in afara grilei!");
    }

    public int getRows() {
        return this.length;
    }

    public int getCols() {
        return this.width;
    }
    public static Grid generateGrid(int length, int width, Character character, GameGUI gameGUI) {
        Grid grid = new Grid(length, width, character, gameGUI);
        Random random = new Random();

        grid.currentX = random.nextInt(length);
        grid.currentY = random.nextInt(width);
        grid.get(grid.currentX).get(grid.currentY).setType(CellEntityType.PLAYER);

        int pX, pY;
        do {
            pX = random.nextInt(length);
            pY = random.nextInt(width);
        } while (pX == grid.currentX && pY == grid.currentY);
        grid.get(pX).get(pY).setType(CellEntityType.PORTAL);

        for (int i = 0; i < 4; i++) {
            int enemyX, enemyY;
            do {
                enemyX = random.nextInt(length);
                enemyY = random.nextInt(width);
            } while (grid.get(enemyX).get(enemyY).getType() != CellEntityType.N);
            grid.get(enemyX).get(enemyY).setType(CellEntityType.ENEMY);
        }

        for (int i = 0; i < 2; i++) {
            int sX, sY;
            do {
                sX = random.nextInt(length);
                sY = random.nextInt(width);
            } while (grid.get(sX).get(sY).getType() != CellEntityType.N);
            grid.get(sX).get(sY).setType(CellEntityType.SANCTUARY);
        }

        return grid;
    }

    public void goNorth() throws ImpossibleMove {
        if (currentX == 0) {
            throw new ImpossibleMove("Nu te poti muta mai sus, esti la margine deja.");
        }
        updateCurrentCell(currentX - 1, currentY);
    }

    public void goSouth() throws ImpossibleMove {
        if (currentX == length - 1) {
            throw new ImpossibleMove("Nu te poti muta mai jos, esti la margine deja.");
        }
        updateCurrentCell(currentX + 1, currentY);
    }

    public void goWest() throws ImpossibleMove {
        if (currentY == 0) {
            throw new ImpossibleMove("Nu te poti muta la stanga, esti la margine deja.");
        }
        updateCurrentCell(currentX, currentY - 1);
    }

    public void goEast() throws ImpossibleMove {
        if (currentY == width - 1) {
            throw new ImpossibleMove("Nu te poti muta la dreapta, esti la margine deja.");
        }
        updateCurrentCell(currentX, currentY + 1);
    }

    private void updateCurrentCell(int newX, int newY) throws ImpossibleMove {
        Cell currentCell = this.get(newX).get(newY);

        this.get(currentX).get(currentY).setType(CellEntityType.VOID);

        if (currentCell.getType() == CellEntityType.SANCTUARY) {
            playerCharacter.increaseHealth(30);
            playerCharacter.increaseMana(40);
            JOptionPane.showMessageDialog(null, "Huh! Ai ajuns într-un sanctuar!\nOdihnește-te puțin pentru a-ți recupera viața și mana.");
        } else if (currentCell.getType() == CellEntityType.PORTAL) {
            JOptionPane.showMessageDialog(null, "Ai ajuns într-un portal! Urmează teleportarea într-o nouă dimensiune..");
           playerCharacter.gainExperience(5 * playerCharacter.getLevel());
            playerCharacter.levelUp();
            resetGame();
            return;
        } else if (currentCell.getType() == CellEntityType.ENEMY) {
            FightPanel(new Enemy());
            if (playerCharacter.getHealth() > 0) {
                JOptionPane.showMessageDialog(null, "Ai doborât inamicul!");
                playerCharacter.gainExperience(20);
                currentCell.setType(CellEntityType.VOID);
            } else {
                JOptionPane.showMessageDialog(null, "Ai fost doborât!\nAi pierdut!");
                System.exit(0);
            }
        }

        currentX = newX;
        currentY = newY;
        currentCell.setType(CellEntityType.PLAYER);
    }

    public void FightPanel(Enemy enemy) {
        JPanel fightPanel = new JPanel(new BorderLayout());

        JLabel enemyHealthLabel = new JLabel("Sănătate inamic: " + enemy.getCurrentHealth());
        JLabel enemyManaLabel = new JLabel("Mana inamic: " + enemy.getCurrentMana());
        JLabel playerHealthLabel = new JLabel("Sănătatea ta: " + playerCharacter.getHealth());
        JLabel playerManaLabel = new JLabel("Mana ta: " + playerCharacter.getMana());

        JPanel actionsPanel = new JPanel(new GridLayout(1, 2));
        JButton basicAttackButton = new JButton("Atac de bază");
        JButton specialAbilityButton = new JButton("Abilitate specială");

        basicAttackButton.addActionListener(e -> {
            enemy.receiveDamage(playerCharacter.calculateDamage());
            enemyHealthLabel.setText("Sănătate inamic: " + enemy.getCurrentHealth());
            enemyManaLabel.setText("Mana inamic: " + enemy.getCurrentMana());

            if (enemy.getCurrentHealth() > 0) {
                playerCharacter.decreaseHealth(enemy.getDamage());
                playerHealthLabel.setText("Sănătatea ta: " + playerCharacter.getHealth());
                playerManaLabel.setText("Mana ta: " + playerCharacter.getMana());
            }
            checkFightStatus(enemy);
        });

        specialAbilityButton.addActionListener(e -> {
            if (mana >= 10) {
                mana -= 10;
                enemyHealthLabel.setText("Sănătate inamic: " + enemy.getCurrentHealth());
                enemyManaLabel.setText("Mana inamic: " + enemy.getCurrentMana());

                if (enemy.getCurrentHealth() > 0) {
                    playerCharacter.decreaseHealth(enemy.getDamage());
                    playerHealthLabel.setText("Sănătatea ta: " + playerCharacter.getHealth());
                    playerManaLabel.setText("Mana ta: " + playerCharacter.getMana());
                }
            } else {
                JOptionPane.showMessageDialog(null, "Mana insuficientă!", "Atenție", JOptionPane.WARNING_MESSAGE);
            }
            checkFightStatus(enemy);
        });

        actionsPanel.add(basicAttackButton);
        actionsPanel.add(specialAbilityButton);

        JPanel statusPanel = new JPanel(new GridLayout(4, 1));
        statusPanel.add(enemyHealthLabel);
        statusPanel.add(enemyManaLabel);
        statusPanel.add(playerHealthLabel);
        statusPanel.add(playerManaLabel);

        fightPanel.add(statusPanel, BorderLayout.CENTER);
        fightPanel.add(actionsPanel, BorderLayout.SOUTH);

        JFrame fightFrame = new JFrame("Luptă");
        fightFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        fightFrame.add(fightPanel);
        fightFrame.pack();
        fightFrame.setVisible(true);
    }

    private void checkFightStatus(Enemy enemy) {
        if (playerCharacter.getHealth()<= 0) {
            JOptionPane.showMessageDialog(null, "Ai fost învins! ", "Înfrângere", JOptionPane.ERROR_MESSAGE);
            gameGUI.FinalPage();
        } else if (enemy.getCurrentHealth() <= 0) {
            JOptionPane.showMessageDialog(null, "Ai învins inamicul!", "Victorie", JOptionPane.INFORMATION_MESSAGE);
            playerCharacter.gainExperience(20);
        }
    }

    public int getPlayerHealth() {
        return playerCharacter.getHealth();
    }
    public int getPlayerMana() {
        return playerCharacter.getMana();
    }
    public int getPlayerExperience() {
        return playerCharacter.getExperience();
    }
    public int getPlayerLevel() {
        return playerCharacter.getLevel();
    }
    public void resetGame() {
        Grid newGrid = generateGrid(length, width, playerCharacter,gameGUI);
        this.clear();
        this.addAll(newGrid);
        currentX = newGrid.currentX;
        currentY = newGrid.currentY;
    }
}