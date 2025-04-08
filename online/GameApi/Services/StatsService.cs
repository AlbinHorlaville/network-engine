using StackExchange.Redis;
using System.Text.Json;

namespace GameApi.Services
{

    public class PlayerStats
    {
        public int GamesWon { get; set; }
        public int CubesPushed { get; set; }
        public int MaxCubesPushedInOneGame { get; set; }
    }

    public class StatsService
    {
        private readonly IDatabase _db;

        public StatsService(IConnectionMultiplexer redis)
        {
            _db = redis.GetDatabase();
        }

        public async Task SetStatsAsync(string username, PlayerStats stats)
        {
            var json = JsonSerializer.Serialize(stats);
            await _db.HashSetAsync("stats", username, json);
        }

        public async Task<PlayerStats?> GetStatsAsync(string username)
        {
            var json = await _db.HashGetAsync("stats", username);
            return json.HasValue ? JsonSerializer.Deserialize<PlayerStats>(json!) : null;
        }
    }
}
