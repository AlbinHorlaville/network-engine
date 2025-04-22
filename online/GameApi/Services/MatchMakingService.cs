using Microsoft.AspNetCore.Mvc;
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
            int tier = Math.Min(gamesWon / 10, 10);
            await _db.ListRightPushAsync($"queue:tier:{tier}", username);
        }

        public async Task<bool> UnqueuePlayerAsync(string username, int tier)
        {
            long removed = await _db.ListRemoveAsync($"queue:tier:{tier}", username);
            if (removed > 0)
                return true;

            return false; // Player was not found in any queue
        }

        public async Task RegisterServerAsync(string ip)
        {
            await _db.SetAddAsync("servers", ip);
        }

        public async Task UnregisterServerAsync(string ip)
        {
            await _db.SetRemoveAsync("servers", ip);
        }

        public async Task<string?> GetMatchStatusAsync(string username)
        {
            var match = await _db.StringGetAsync($"player:{username}:match");
            return match;
        }

        public async Task RemovePlayerFromMatchAsync(string username)
        {
            await _db.KeyDeleteAsync($"player:{username}:match");
        }
    }
}
