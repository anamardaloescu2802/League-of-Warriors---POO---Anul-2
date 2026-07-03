import javax.swing.*;
import java.awt.*;
import java.util.List;
import entities.Account;
import entities.characters.Character;

public class GameGUI extends JFrame {
    private Account currentAccount;
    private Character selectedCharacter;
    private Grid grid;
    private JPanel gridDisplay;
    private JLabel statusLabel;
    private JPanel mainPanel;
    private JFrame fightFrame;

    private static final Dimension PANEL_SIZE = new Dimension(1200, 700);
    private static final Dimension PANEL_SIZE2 = new Dimension(800, 400);
    private static final Dimension FRAME_SIZE = new Dimension(1200, 700);
    private static final Dimension CONTROL_BUTTON_SIZE = new Dimension(120, 40);
    private static GameGUI instance;

    private GameGUI() {
        setTitle("League of Warriors");
        setSize(FRAME_SIZE);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        setLayout(new BorderLayout());

        UIManager.put("Panel.background", new Color(34, 34, 34));
        UIManager.put("Button.background", new Color(44, 62, 80));
        UIManager.put("Button.foreground", Color.WHITE);
        UIManager.put("Button.font", new Font("Verdana", Font.BOLD, 14));
        UIManager.put("Label.foreground", new Color(241, 196, 15));
        UIManager.put("TextArea.background", new Color(52, 73, 94));
        UIManager.put("TextArea.foreground", Color.WHITE);
        UIManager.put("TextArea.font", new Font("Monospaced", Font.PLAIN, 20));
        UIManager.put("ScrollPane.border", BorderFactory.createLineBorder(new Color(100, 100, 100), 2));

        mainPanel = createMainPanel();
        add(mainPanel, BorderLayout.CENTER);

        login();
    }
    public static GameGUI getInstance() {
        if (instance == null) {
            synchronized (GameGUI.class) {
                if (instance == null) {
                    instance = new GameGUI();
                }
            }
        }
        return instance;
    }
    private JPanel createMainPanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(0, 0, 0, 0);
        gbc.fill = GridBagConstraints.BOTH;
        gbc.gridx = 0;
        gbc.gridy = 0;

        statusLabel = new JLabel("", JLabel.CENTER);
        statusLabel.setFont(new Font("Arial", Font.BOLD, 20));
        statusLabel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
        gbc.gridy = 0;
        gbc.weightx = 1;
        gbc.weighty = 0;
        panel.add(statusLabel, gbc);

        gridDisplay = new JPanel();
        gridDisplay.setLayout(new GridLayout(5, 5, 0, 0));
        gridDisplay.setPreferredSize(PANEL_SIZE2);
        gridDisplay.setOpaque(true);
        gridDisplay.setBackground(Color.BLACK);

        JScrollPane scrollPane = new JScrollPane(gridDisplay);
        scrollPane.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
        scrollPane.setOpaque(false);
        scrollPane.getViewport().setOpaque(false);

        gbc.gridy = 1;
        gbc.weighty = 1;
        panel.add(scrollPane, gbc);

        gbc.gridy = 2;
        gbc.weighty = 0;
        panel.add(createControlPanel(), gbc);

        return panel;
    }

    private JPanel createControlPanel() {
        JPanel controlPanel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;
        gbc.insets = new Insets(5, 5, 5, 5);

        JButton northButton = createStyledButton("Sus");
        JButton southButton = createStyledButton("Jos");
        JButton westButton = createStyledButton("Stanga");
        JButton eastButton = createStyledButton("Dreapta");
        JButton quitButton = createStyledButton("Paraseste jocul");

        northButton.addActionListener(e -> movePlayer("N"));
        southButton.addActionListener(e -> movePlayer("S"));
        westButton.addActionListener(e -> movePlayer("W"));
        eastButton.addActionListener(e -> movePlayer("E"));
        quitButton.addActionListener(e -> FinalPage());

        gbc.gridx = 1;
        gbc.gridy = 0;
        controlPanel.add(northButton, gbc);

        gbc.gridx = 0;
        gbc.gridy = 1;
        controlPanel.add(westButton, gbc);

        gbc.gridx = 2;
        gbc.gridy = 1;
        controlPanel.add(eastButton, gbc);

        gbc.gridx = 1;
        gbc.gridy = 2;
        controlPanel.add(southButton, gbc);

        gbc.gridx = 0;
        gbc.gridy = 3;
        gbc.gridwidth = 3;
        controlPanel.add(quitButton, gbc);

        return controlPanel;
    }

    private JButton createStyledButton(String text) {
        JButton button = new JButton(text);
        button.setFocusPainted(false);
        button.setFont(new Font("Verdana", Font.BOLD, 14));
        button.setPreferredSize(CONTROL_BUTTON_SIZE);
        button.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(100, 100, 100), 2),
                BorderFactory.createEmptyBorder(5, 15, 5, 15))
        );
        button.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseEntered(java.awt.event.MouseEvent evt) {
                button.setBackground(new Color(52, 152, 219));
            }

            public void mouseExited(java.awt.event.MouseEvent evt) {
                button.setBackground(new Color(44, 62, 80));
            }
        });
        return button;
    }

    private void login() {
        BackgroundPanel loginPanel = new BackgroundPanel("C:\\Users\\anama\\IdeaProjects\\tema2\\src\\fundal.jpeg");
        loginPanel.setLayout(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(10, 10, 10, 10);
        gbc.fill = GridBagConstraints.HORIZONTAL;

        JLabel titleLabel = new JLabel("League of Warriors");
        titleLabel.setFont(new Font("Algerian", Font.BOLD, 50));
        titleLabel.setForeground(Color.YELLOW);
        titleLabel.setHorizontalAlignment(JLabel.CENTER);
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.gridwidth = 2;
        loginPanel.add(titleLabel, gbc);

        JLabel emailLabel = new JLabel("Email:");
        JLabel passwordLabel = new JLabel("Parola:");
        JTextField emailField = new JTextField();
        JPasswordField passwordField = new JPasswordField();

        emailLabel.setForeground(Color.YELLOW);
        passwordLabel.setForeground(Color.YELLOW);

        gbc.gridx = 0;
        gbc.gridy = 1;
        gbc.gridwidth = 1;
        loginPanel.add(emailLabel, gbc);
        gbc.gridx = 1;
        gbc.gridy = 1;
        loginPanel.add(emailField, gbc);

        gbc.gridx = 0;
        gbc.gridy = 2;
        loginPanel.add(passwordLabel, gbc);
        gbc.gridx = 1;
        gbc.gridy = 2;
        loginPanel.add(passwordField, gbc);

        emailField.setPreferredSize(new Dimension(200, 30));
        passwordField.setPreferredSize(new Dimension(200, 30));

        int result = JOptionPane.showConfirmDialog(this, loginPanel, "Autentificare", JOptionPane.OK_CANCEL_OPTION);
        if (result == JOptionPane.OK_OPTION) {
            String email = emailField.getText();
            String password = new String(passwordField.getPassword());
            currentAccount = authenticate(email, password);

            if (currentAccount == null) {
                JOptionPane.showMessageDialog(this, "Email sau parolă greșită!", "Eroare", JOptionPane.ERROR_MESSAGE);
                System.exit(0);
            }

            selectedCharacter = displayCharactersAndSelect();

            if (selectedCharacter == null) {
                JOptionPane.showMessageDialog(this, "Nu se poate începe jocul!", "Eroare", JOptionPane.ERROR_MESSAGE);
                restartGame();
            }

            grid = Grid.generateGrid(5, 5, selectedCharacter, this);
            updateStatusLabel();
            mainPanel.revalidate();
            mainPanel.repaint();
            updateGridDisplay();
        } else {
            System.exit(0);
        }
    }

    class BackgroundPanel extends JPanel {
        private Image backgroundImage;

        public BackgroundPanel(String imagePath) {
            backgroundImage = new ImageIcon(imagePath).getImage();
            setPreferredSize(PANEL_SIZE);
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            g.drawImage(backgroundImage, 0, 0, PANEL_SIZE.width, PANEL_SIZE.height, this);
        }
    }

    private Account authenticate(String email, String password) {
        List<Account> accounts = JsonInput.deserializeAccounts();
        for (Account account : accounts) {
            if (account.getInformation().getCredentials().getEmail().equals(email) &&
                    account.getInformation().getCredentials().getPassword().equals(password)) {
                return account;
            }
        }
        return null;
    }

    private Character displayCharactersAndSelect() {
        List<Character> characters = currentAccount.getCharacters();
        if (characters.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Nu există niciun caracter disponibil.", "Eroare", JOptionPane.ERROR_MESSAGE);
            return null;
        }

        JPanel characterPanel = new JPanel();
        characterPanel.setLayout(new BoxLayout(characterPanel, BoxLayout.Y_AXIS));
        characterPanel.setOpaque(false);
        ButtonGroup group = new ButtonGroup();

        for (Character character : characters) {
            JPanel characterInfoPanel = new JPanel();
            characterInfoPanel.setLayout(new BoxLayout(characterInfoPanel, BoxLayout.Y_AXIS));
            characterInfoPanel.setOpaque(false);
            characterInfoPanel.setAlignmentX(Component.CENTER_ALIGNMENT);

            JLabel levelAndExpLabel = new JLabel("Nivel: " + character.getLevel() + " | Experiența: " + character.getExperience());
            levelAndExpLabel.setFont(new Font("Verdana", Font.PLAIN, 16));
            levelAndExpLabel.setForeground(Color.WHITE);
            levelAndExpLabel.setAlignmentX(Component.CENTER_ALIGNMENT);

            JLabel nameAndTypeLabel = new JLabel(character.getName() + " - " + character.getType());
            nameAndTypeLabel.setFont(new Font("Algerian", Font.BOLD, 30));
            nameAndTypeLabel.setForeground(Color.LIGHT_GRAY);
            nameAndTypeLabel.setAlignmentX(Component.CENTER_ALIGNMENT);

            JRadioButton radioButton = new JRadioButton();
            radioButton.setOpaque(false);
            radioButton.setAlignmentX(Component.CENTER_ALIGNMENT);
            group.add(radioButton);

            characterInfoPanel.add(levelAndExpLabel);
            characterInfoPanel.add(nameAndTypeLabel);
            characterInfoPanel.add(radioButton);

            characterPanel.add(characterInfoPanel);
            characterPanel.add(Box.createVerticalStrut(10));
        }

        JScrollPane scrollPane = new JScrollPane(characterPanel);
        scrollPane.setPreferredSize(PANEL_SIZE);
        scrollPane.setOpaque(false);
        scrollPane.getViewport().setOpaque(false);

        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.setOpaque(false);

        mainPanel.add(scrollPane, BorderLayout.CENTER);

        JLabel readyLabel = new JLabel("Incepem?", JLabel.CENTER);
        readyLabel.setFont(new Font("Verdana", Font.BOLD, 18));
        readyLabel.setForeground(new Color(255, 255, 0));
        readyLabel.setBorder(BorderFactory.createEmptyBorder(10, 0, 10, 0));
        mainPanel.add(readyLabel, BorderLayout.SOUTH);

        int result = JOptionPane.showConfirmDialog(this, mainPanel, "Selectare Caracter", JOptionPane.OK_CANCEL_OPTION);
        if (result == JOptionPane.OK_OPTION) {
            for (Component comp : characterPanel.getComponents()) {
                if (comp instanceof JPanel) {
                    for (Component innerComp : ((JPanel) comp).getComponents()) {
                        if (innerComp instanceof JRadioButton && ((JRadioButton) innerComp).isSelected()) {
                            JLabel nameAndTypeLabel = (JLabel) ((JPanel) comp).getComponent(1);
                            String selectedName = nameAndTypeLabel.getText().split(" - ")[0];
                            return characters.stream().filter(c -> c.getName().equals(selectedName)).findFirst().orElse(null);
                        }
                    }
                }
            }
        }

        return null;
    }

    private void movePlayer(String direction) {
        try {
            switch (direction) {
                case "N":
                    grid.goNorth();
                    break;
                case "S":
                    grid.goSouth();
                    break;
                case "W":
                    grid.goWest();
                    break;
                case "E":
                    grid.goEast();
                    break;
            }
            handleCellEvent();
            updateStatusLabel();
            updateGridDisplay();
        } catch (ImpossibleMove e) {
            JOptionPane.showMessageDialog(this, e.getMessage(), "Mutare imposibilă", JOptionPane.WARNING_MESSAGE);
        }
    }

    private void handleCellEvent() {
        Cell currentCell = grid.get(grid.currentX).get(grid.currentY);
        switch (currentCell.getType()) {
            case SANCTUARY:
                JOptionPane.showMessageDialog(this, "Ai ajuns într-un sanctuar. Viața și mana au fost restaurate.", "Sanctuar", JOptionPane.INFORMATION_MESSAGE);
                updateStatusLabel();
                break;
            case PORTAL:
                JOptionPane.showMessageDialog(this, "Ai ajuns într-un portal!", "Portal", JOptionPane.INFORMATION_MESSAGE);
                updateStatusLabel();
                grid.resetGame();
                break;
            case ENEMY:
                showFightPanel(new Enemy());
                updateStatusLabel();
                break;
        }
    }


    public void FinalPage() {
        getContentPane().removeAll();
        JPanel finalPage = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(40, 40, 40, 40);

        JLabel finalLabel1 = new JLabel("Ne pare rau ca s-a terminat asa..");
        finalLabel1.setFont(new Font("Algerian", Font.BOLD, 30));
        finalLabel1.setForeground(Color.YELLOW);

        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.gridwidth = 2;
        gbc.anchor = GridBagConstraints.CENTER;
        finalPage.add(finalLabel1, gbc);

        JLabel finalLabel2 = new JLabel("Ce vei face acum?");
        finalLabel2.setFont(new Font("Algerian", Font.BOLD, 30));
        finalLabel2.setForeground(Color.YELLOW);

        gbc.gridy = 1;
        finalPage.add(finalLabel2, gbc);

        JButton exitButton = createStyledButton("Inchid");
        exitButton.addActionListener(e -> System.exit(0));

        JButton selectCharacterButton = createStyledButton("Restart");
        selectCharacterButton.addActionListener(e -> {
            getContentPane().removeAll();
            selectedCharacter = null;
            grid = null;
            mainPanel = createMainPanel();
            add(mainPanel, BorderLayout.CENTER);
            selectedCharacter = displayCharactersAndSelect();

            if (selectedCharacter != null) {
                grid = Grid.generateGrid(5, 5, selectedCharacter, this);
                updateStatusLabel();
                updateGridDisplay();
            } else {
                JOptionPane.showMessageDialog(this, "Nu s-a putut selecta un caracter nou. Jocul se va închide.", "Eroare", JOptionPane.ERROR_MESSAGE);
                System.exit(0);
            }

            revalidate();
            repaint();
        });

        gbc.gridwidth = 1;
        gbc.gridy = 2;
        gbc.gridx = 0;
        gbc.anchor = GridBagConstraints.CENTER;
        finalPage.add(exitButton, gbc);

        gbc.gridx = 1;
        finalPage.add(selectCharacterButton, gbc);

        add(finalPage, BorderLayout.CENTER);
        revalidate();
        repaint();
    }


    private void showFightPanel(Enemy enemy) {
        JPanel fightPanel = new JPanel(new BorderLayout());
        fightPanel.setOpaque(true);
        fightPanel.setPreferredSize(PANEL_SIZE);
        JLabel enemyHealthLabel = new JLabel("Săntate inamic: " + enemy.getCurrentHealth());
        JLabel playerHealthLabel = new JLabel("Săntatea ta: " + selectedCharacter.getHealth());

        JButton attackButton = createStyledButton("Atac");
        attackButton.addActionListener(e -> {
            enemy.receiveDamage(selectedCharacter.calculateDamage());
            enemyHealthLabel.setText("Săntate inamic: " + enemy.getCurrentHealth());

            if (enemy.getCurrentHealth() <= 0) {
                JOptionPane.showMessageDialog(this, "Ai învins inamicul!", "Victorie", JOptionPane.INFORMATION_MESSAGE);
                updateStatusLabel();
                mainPanel.revalidate();
                mainPanel.repaint();

            } else {
                selectedCharacter.decreaseHealth(enemy.getDamage());
                playerHealthLabel.setText("Săntatea ta: " + selectedCharacter.getHealth());

                if (selectedCharacter.getHealth() <= 0) {
                    mainPanel.revalidate();
                    mainPanel.repaint();
                }
            }});

        fightPanel.add(enemyHealthLabel, BorderLayout.NORTH);
        fightPanel.add(playerHealthLabel, BorderLayout.CENTER);
        fightPanel.add(attackButton, BorderLayout.SOUTH);

        fightFrame = new JFrame("Lupta");
        fightFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        fightFrame.setSize(FRAME_SIZE);
        fightFrame.add(fightPanel);
        fightFrame.setVisible(true);
        mainPanel.revalidate();
        mainPanel.repaint();
    }

    public void restartGame() {
        selectedCharacter = displayCharactersAndSelect();
        if (selectedCharacter != null) {
            grid = Grid.generateGrid(5, 5, selectedCharacter, this);
            updateStatusLabel();
            mainPanel.revalidate();
            mainPanel.repaint();

            updateGridDisplay();
        } else {
            JOptionPane.showMessageDialog(this, "Nu s-a putut selecta un caracter nou. Jocul se va închide.", "Eroare", JOptionPane.ERROR_MESSAGE);
            System.exit(0);
        }
    }

    private void updateStatusLabel() {
        String statusText = "Mana: " + grid.getPlayerMana() + " | Viață: " + grid.getPlayerHealth()+
                " | Nivel: " + grid.getPlayerLevel() +
                " | Experiență: " + grid.getPlayerExperience();
        statusLabel.setText(statusText);
    }


    private ImageIcon getCharacterIcon(Character character) {
        String iconPath;
        switch (character.getType()) {
            case "Warrior":
                iconPath = "C:\\Users\\anama\\IdeaProjects\\tema2\\src\\Warrior.PNG";
                break;
            case "Mage":
                iconPath = "C:\\Users\\anama\\IdeaProjects\\tema2\\src\\Mage.PNG";
                break;
            case "Rogue":
                iconPath = "C:\\Users\\anama\\IdeaProjects\\tema2\\src\\Rogue.PNG";
                break;
            default:
                iconPath = "C:\\Users\\anama\\IdeaProjects\\tema2\\src\\Warrior.PNG";
                break;
        }
        return resizeImageIcon(new ImageIcon(iconPath), 80, 80);
    }

    private void updateGridDisplay() {
        gridDisplay.removeAll();

        for (List<Cell> row : grid) {
            for (Cell cell : row) {
                JLabel cellLabel = new JLabel();
                cellLabel.setBorder(BorderFactory.createEmptyBorder());
                cellLabel.setOpaque(false);

                ImageIcon icon;

                switch (cell.getType()) {
                    case PLAYER:
                        icon = getCharacterIcon(selectedCharacter);
                        break;
                    case SANCTUARY:

                        icon = resizeImageIcon(new ImageIcon("C:\\Users\\anama\\IdeaProjects\\tema2\\src\\poza.png"), 25, 25);
                        break;
                    case PORTAL:
                        icon = resizeImageIcon(new ImageIcon("C:\\Users\\anama\\IdeaProjects\\tema2\\src\\poza.png"), 25, 25);
                        break;
                    case VOID:
                        icon = resizeImageIcon(new ImageIcon("C:\\Users\\anama\\IdeaProjects\\tema2\\src\\void.png"), 25, 25);
                        break;
                        default:
                        icon = resizeImageIcon(new ImageIcon("C:\\Users\\anama\\IdeaProjects\\tema2\\src\\poza.png"), 25, 25);
                        break;
                }

                cellLabel.setIcon(icon);
                cellLabel.setHorizontalAlignment(JLabel.CENTER);
                gridDisplay.add(cellLabel);
            }
        }

        gridDisplay.revalidate();
        gridDisplay.repaint();
    }


    private ImageIcon resizeImageIcon(ImageIcon icon, int width, int height) {
        Image image = icon.getImage();
        Image resizedImage = image.getScaledInstance(width, height, Image.SCALE_SMOOTH);
        return new ImageIcon(resizedImage);
    }
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            GameGUI gameGUI = GameGUI.getInstance();
            gameGUI.setVisible(true);
        });
    }


}