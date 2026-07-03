package entities;

import java.util.SortedSet;

public class Information {
    private String name;
    private String country;
    private Credentials credentials;
    private SortedSet<String> favoriteGames;

    // Constructor privat
    private Information(Builder builder) {
        this.name = builder.name;
        this.country = builder.country;
        this.credentials = builder.credentials;
        this.favoriteGames = builder.favoriteGames;
    }

    // Getter pentru câmpuri
    public String getName() {
        return name;
    }

    public String getCountry() {
        return country;
    }

    public Credentials getCredentials() {
        return credentials;
    }

    public SortedSet<String> getFavoriteGames() {
        return favoriteGames;
    }

    // Builder pentru a crea instanțe ale clasei Information
    public static class Builder {
        private String name;
        private String country;
        private Credentials credentials;
        private SortedSet<String> favoriteGames;

        public Builder name(String name) {
            this.name = name;
            return this;
        }

        public Builder country(String country) {
            this.country = country;
            return this;
        }

        public Builder credentials(Credentials credentials) {
            this.credentials = credentials;
            return this;
        }

        public Builder favoriteGames(SortedSet<String> favoriteGames) {
            this.favoriteGames = favoriteGames;
            return this;
        }

        public Information build() {
            return new Information(this);
        }
    }
}
