using StackExchange.Redis;
using System.Text.Json;

namespace GameApi.Services
{
    public class MatchmakingService
    {
        private readonly IDatabase _db;

        public MatchmakingService(IConnectionMultiplexer redis)
        {
            _db = redis.GetDatabase();
        }

        public async Task QueuePlayerAsync(string username, int gamesWon)
        {
            int tier = gamesWon / 10;
            await _db.ListRightPushAsync($"queue:tier:{tier}", username);
        }

        public async Task<List<string>> GetNextMatchAsync(int tier)
        {
            var players = await _db.ListLeftPopAsync($"queue:tier:{tier}", 4);
            return players.Select(p => p.ToString()).ToList();
        }

        public async Task RegisterServerAsync(string ip)
        {
            await _db.SetAddAsync("servers", ip);
            await _db.StringSetAsync($"server:{ip}:available", true);
        }

        public async Task<string?> GetAvailableServerAsync()
        {
            var servers = await _db.SetMembersAsync("servers");
            foreach (var server in servers)
            {
                var isAvailable = await _db.StringGetAsync($"server:{server}:available");
                if (isAvailable == "True")
                {
                    await _db.StringSetAsync($"server:{server}:available", false);
                    return server.ToString();
                }
            }

            return null;
        }

        public async Task MarkServerAvailableAsync(string ip)
        {
            await _db.StringSetAsync($"server:{ip}:available", true);
        }
    }
}
