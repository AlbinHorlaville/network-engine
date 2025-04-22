using GameApi.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.IdentityModel.Tokens;
using StackExchange.Redis;
using System.Text.RegularExpressions;

namespace GameApi.Controllers
{
    [ApiController]
    [Authorize]
    [Route("[controller]")]
    public class MatchmakingController : ControllerBase
    {
        private readonly MatchmakingService _matchService;
        private readonly StatsService _statsService;

        public MatchmakingController(MatchmakingService matchService, StatsService statsService)
        {
            _matchService = matchService;
            _statsService = statsService;
        }

        [HttpPost("queue")]
        public async Task<IActionResult> QueuePlayer()
        {
            var username = User.Identity?.Name!;
            var stats = await _statsService.GetStatsAsync(username);
            var wins = stats?.GamesWon ?? 0;
            await _matchService.QueuePlayerAsync(username, wins);
            return Ok("Player queued");
        }

        [HttpPost("unqueue")]
        public async Task<IActionResult> UnqueuePlayer()
        {
            var username = User.Identity?.Name!;
            var stats = await _statsService.GetStatsAsync(username);
            var wins = stats?.GamesWon ?? 0;
            int tier = Math.Min(wins / 10, 10);
            var success = await _matchService.UnqueuePlayerAsync(username, tier);

            if (!success)
                return NotFound("Player was not in queue.");

            return Ok("Player unqueued");
        }

        [HttpPost("register")]
        public async Task<IActionResult> RegisterServer([FromQuery] string ip)
        {
            await _matchService.RegisterServerAsync(ip);
            return Ok("Server registered");
        }

        [HttpPost("unregister")]
        public async Task<IActionResult> UnregisterServer([FromQuery] string ip)
        {
            await _matchService.UnregisterServerAsync(ip);
            return Ok("Server unregistered");
        }

        [HttpGet("status")]
        public async Task<IActionResult> GetMatchStatus()
        {
            var username = User.Identity?.Name!;
            string? matchFound = await _matchService.GetMatchStatusAsync(username);

            if (matchFound.IsNullOrEmpty())
            {
                return NotFound("No match found");
            } else
            {
                return Ok(new { Server = matchFound });
            }
 
        }

        [HttpDelete("match/{username}")]
        public async Task<IActionResult> RemovePlayerFromMatch(string username)
        {
            await _matchService.RemovePlayerFromMatchAsync(username);
            return NoContent(); // 204 No Content is common for delete success
        }
    }
}
