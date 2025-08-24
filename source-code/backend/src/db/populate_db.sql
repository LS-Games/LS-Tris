PRAGMA foreign_keys = ON;

INSERT INTO Player (nickname, email, password, current_streak, max_streak, registration_date) VALUES
('AlphaWolf', 'alpha@example.com', 'pass123', 2, 5, '2025-01-15'),
('BetaGamer', 'beta@example.com', 'beta456', 0, 3, '2025-02-20'),
('CharliePro', 'charlie@example.com', 'charlie789', 1, 4, '2025-03-05'),
('DeltaQueen', 'delta@example.com', 'delta321', 5, 6, '2025-03-10');

INSERT INTO Game (id_creator, id_owner, state, created_at) VALUES
(1, 2, 'new', '2025-08-01 10:00:00'),
(2, 3, 'active', '2025-08-02 15:30:00'),
(3, 4, 'waiting', '2025-08-03 18:45:00'),
(4, 1, 'finished', '2025-08-04 20:15:00');

INSERT INTO Round (id_game, state, duration, board) VALUES
(1, 'pending', 60, 'X__-XOO-X__'),
(2, 'active', 120, 'XOO-XOO-X__'),
(2, 'finished', 90, 'XOO-XOO-XXX'),
(4, 'finished', 75, '___-___-___');

INSERT INTO Play (id_player, id_round, result) VALUES
(1, 1, 'win'),
(2, 1, 'lose'),
(2, 2, 'win'),
(3, 2, 'lose'),
(2, 3, 'draw'),
(3, 3, 'draw'),
(4, 4, 'win'),
(1, 4, 'lose');

INSERT INTO Participation_request (id_player, id_game, created_at, state) VALUES
(1, 2, '2025-08-01 11:00:00', 'pending'),
(3, 1, '2025-08-01 12:30:00', 'accepted'),
(4, 3, '2025-08-02 14:00:00', 'rejected'),
(2, 4, '2025-08-03 09:45:00', 'canceled');
