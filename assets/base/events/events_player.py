from engine import create_logger, event_bus, EventPlayer  # noqa

logger = create_logger(__name__)


@event_bus.listen('player_logged_in')
def event_player_logged_in(event: EventPlayer):
    logger.info(f'event_player_logged_in() called with player: {event.player_id}')
