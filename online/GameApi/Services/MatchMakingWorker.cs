using GameApi.Services;
using Microsoft.Extensions.Hosting;
using StackExchange.Redis;

public class MatchMakingWorker : BackgroundService
{
    private readonly IServiceProvider _services;
    private readonly IDatabase _db;

    public MatchMakingWorker(IServiceProvider services, IConnectionMultiplexer redis)
    {
        _services = services;
        _db = redis.GetDatabase();
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        try
        {
            while (!stoppingToken.IsCancellationRequested)
            {
                using var scope = _services.CreateScope();
                var matchService = scope.ServiceProvider.GetRequiredService<MatchmakingService>();

                var serversTest = await _db.SetMembersAsync("servers");
                foreach (var server in serversTest)
                {
                    Console.WriteLine(server);
                }

                for (int tier = 0; tier <= 10; tier++)
                {
                    var playersRedisValue = await _db.ListLeftPopAsync($"queue:tier:{tier}", 4);

                    if (playersRedisValue is null || playersRedisValue.Length == 0)
                        continue; // Nothing to match in this tier

                    List<string> players = playersRedisValue.Select(p => p.ToString()).ToList();

                    foreach(string player in players)
                    {
                        Console.WriteLine(player);
                    }

                    if (players.Count == 4)
                    {
                        var chosenServer = "";
                        var servers = await _db.SetMembersAsync("servers");
                        foreach (var server in servers)
                        {
                            chosenServer = server.ToString();
                            await _db.SetRemoveAsync("servers", chosenServer);
                            break;
                        }

                        if (chosenServer != "")
                        {
                            Console.WriteLine($"Match found for tier {tier} on server {chosenServer}: {string.Join(", ", players)}");
                            foreach (var player in players)
                            {
                                await _db.StringSetAsync($"player:{player}:match", chosenServer);
                            }
                        }
                        else
                        {
                            Console.WriteLine("No available server for match.");
                            // Re-queue players back to the front of the same tier queue
                            foreach (var player in players)
                            {
                                await _db.ListLeftPushAsync($"queue:tier:{tier}", player);
                            }
                        }
                    } else {
                        Console.WriteLine("Not enough players for match.");
                        // Re-queue players back to the front of the same tier queue
                        foreach (var player in players)
                        {
                            await _db.ListLeftPushAsync($"queue:tier:{tier}", player);
                        }
                    }
                }

                await Task.Delay(3000); // Wait 3s before checking again
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"MatchMakingWorker crashed: {ex.Message}");
        }
    }
}